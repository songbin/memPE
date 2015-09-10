#include "mkv_thread.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "mkvdb.h"
#include "errdesc.h"
#include <memcached.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "msg_queue.h"

#define USE_QUEUE_LIST //use the queue which we designed instead of linux's msg queue



typedef struct DBItem{
    item *item;
    enum process_type action;
    struct DBItem *next;
}DBITEM;

typedef struct DBItemQueue{
    DBITEM *head;
    DBITEM *tail;
    pthread_mutex_t lock;
}DBIQ;

#define ITEMS_PER_ALLOC 128

static bool bLoop = false;
static unsigned char g_key[KEY_MAX_LENGTH] = {0x0};
static LIBEVENT_THREAD mkv_thread;

static int _save_item(item *item);
static int _del_item(item *item);
static int _item_arithmetic(item *item);
static int _item_flush();
static int _item_touch(item *item);
static int _update_database(enum process_type action, item *item);

/**@brief process the item[save or delete]
 * @param item [in] the item which will be processed 
 * @param type [int] SAVE_ITEM OR DEL_ITEM
 */
int process_item(item *item, enum process_type type){
    int ret = ERR_OK;

    if(!settings.usePrs)
        return ERR_OK;
    
    if( NULL == item && enum_flush_item != type )
        return ERR_NULL;

    char buf[1] = {0x0}; 
    
    if( 0 != queue_push(item, type) ){
        printf("push failed\n");
        return ERR_FAIL;
    }

    if( !bLoop ){
        buf[0] = FLAG_PRO_ITEM;
        if ( write_pipe( buf, 1 ) != 1 ) {
            perror("Writing to thread notify pipe");
            return ERR_FAIL;
        }
    }

    return ret;
}

/**@brief save the memcache data into database
 *   @param [in] item the item's pointer of slab of memcached 
 *   @return 0 == ok 
 *   @return others == error
*/
int _save_item(item *item){
    if( NULL == item )
        return ERR_NULL;
    
    int flags = 0;
    int size = 0;
    int ret = ERR_OK;
    unsigned char *pValue = NULL;
    
    memset(g_key, 0x0, KEY_MAX_LENGTH);
    memmove(g_key, ITEM_key(item), item->nkey);
    flags = atoi( ITEM_suffix(item) );
    
    if ( ERR_OK != to_string( flags, 
                              item->nbytes, 
                              item->nkey, 
                              item->exptime,
                              ITEM_data(item), 
                              &size,
                              &pValue) ){
        return ERR_FAIL;
    }     

    ret = mkv_set_key( g_key, 
                 pValue, 
                 size, 
                 settings.db); 

    if( size > MAX_CHUNK && pValue ){
        free(pValue);
    }
    
    return ret;
}

int _del_item(item *item){
    int ret =  ERR_OK;
    if( NULL == item )
        return ERR_NULL;

    const unsigned char *key = (unsigned char *)ITEM_key(item);
    ret = mkv_del_key(key, settings.db);
        
    return ret;    
}
/**@brief for incr and decr command
 *   
 */
int _item_arithmetic(item *item){
    return _save_item(item);
}

static int _item_flush(){
    mkv_close_db(settings.db);
    unlink(settings.szDBName);

    settings.db = mkv_init(settings.szDBName);
    if( !settings.db )
        settings.db = mkv_init(settings.szDBName);
    
    return settings.db ? ERR_OK : ERR_DB_INVALID;
}

static int _item_touch(item *item){
    return _save_item(item);
}

int mkv_thread_init(void){
    pthread_t ntid;
    pthread_attr_t attr;
     
    if( 0 != queue_init() ){
        return ERR_FAIL;
    }

    pthread_attr_init( &attr );
    if( 0 != pthread_create(&ntid , &attr , _mkv_start_thread , (void*)&mkv_thread) ){
        return ERR_THREAD_FAILED;
    }

    mkv_thread.thread_id = ntid;

    if ( pipe(mkvFds) ) {
        perror("Can't create notify pipe");
        return ERR_PIPE_FAIL;
    }

    mkv_thread.notify_receive_fd = mkvFds[0];
    mkv_thread.notify_send_fd = mkvFds[1];    
    
    return 0;   
}

void *_mkv_start_thread(void *args){
    if( NULL == args ) 
        return NULL;

    LIBEVENT_THREAD *pthread = (LIBEVENT_THREAD*)args;
    if( ERR_OK != _mkv_setup_thread(pthread) ){
        return NULL;
    }

    return NULL;
}

int _mkv_setup_thread(void *args){
    if( NULL == args ) 
        return ERR_STRING_NULL;
    
    LIBEVENT_THREAD *me = (LIBEVENT_THREAD*)args;
    me->base = event_init();
    if (! me->base) {
        fprintf(stderr, "Can't allocate event base\n");
        return ERR_EVENT_FAILED;
    }

    /* Listen for notifications from other threads */
    event_set(&me->notify_event, me->notify_receive_fd,
              EV_READ | EV_PERSIST, _mkv_process_queue, me);
    
    event_base_set(me->base, &me->notify_event);

    if( event_add(&me->notify_event, 0) == -1 ){
        fprintf(stderr, "Can't monitor libevent notify pipe\n");
        exit(1);
    }

    event_base_loop(me->base, 0); 
    return ERR_OK;
}

void _mkv_process_queue(int fd, short which, void *args){  
    char pipe[20] = {0x0};
    QLItem *qitem = NULL;
    read_pipe( pipe );
    if( pipe[0] == FLAG_PRO_ITEM ){
        bLoop = true;
    }
    
    while(bLoop){       
        qitem = queue_pop();
        if( NULL == qitem ){
            fprintf(stderr, "msg queue is null !\n");  
            bLoop = false;
            continue;
        }
        _update_database(qitem->action,qitem->item);
        free(qitem);
        qitem = NULL;   
    }

    return; 
}


int _read_handle( int fd, char **pBuff, int *size ){
    char szData[1024] = {0x0};
    int cnt = 1;
    int ret = ERR_OK;
    int len = 0;
    *size = 0;

    *pBuff = (char*)calloc( sizeof(char), 1024 );
    if( NULL == pBuff ) 
        return ERR_STRING_NULL;
    
    while( 0 < ( len = read( fd, szData, 1000) ) && cnt > 0 ){
        if( cnt++ > 1 ){
            *pBuff = (char*)realloc( *pBuff, 1024*cnt );
            if( NULL == pBuff ){
                ret = ERR_STRING_NULL;
                break;
            } 
        }
        strncat( *pBuff, szData, 1000 ); 
        *size += len;
    }
    
    return ret;
}

int _write_handle( int fd, char *pBuff, int size ){
    int ret = ERR_OK;

    if( NULL == pBuff || 0 == size ){
        return ERR_OK;        
    }
    
    if( size != write( fd, pBuff, size ) ){
        return ERR_FAIL;
    }
    
    return ret;
}
/**@brief pipe just transit small data , the data size must be smaller than 10 bytes

*/
int read_pipe( char *pBuff ){
    int ret = read( mkvFds[0], pBuff , 10 ) ;  
    return ret;
}

/**@brief pipe just transit small data , the data size must be smaller than 10 bytes

*/
int write_pipe( char *pBuff, int size ){
    if( NULL == pBuff || 0 == size ){
        return ERR_OK;        
    }
    
    if( size != write( mkvFds[1], pBuff, size ) ){
        return ERR_FAIL;
    }

    return ERR_OK;
}

int _update_database(enum process_type action, item *item){
    int ret;
    switch(action){
            case enum_save_item:
                ret = _save_item(item);
                fprintf(stderr, "save[%d]...\n",ret); 
                break;
            case enum_del_item:
                ret = _del_item(item);
                break;
            case enum_item_incr:
            case enum_item_decr:
                ret = _item_arithmetic(item);
                break;
            case enum_flush_item:
                ret = _item_flush();
                break;
            case enum_touch_item:
                ret = _item_touch(item);
                break;
            default:
                printf("default comming\n");
                break;
        }
    return ret;
}

