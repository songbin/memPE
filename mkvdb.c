#include "mkvdb.h"
#include <memcached.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static unsigned char *g_buf ;

static int _block_init(void){
    int ret = ERR_OK;
    
    g_buf = (unsigned char*)calloc(sizeof(char), MAX_CHUNK);
    if( NULL == g_buf )
        return ERR_STRING_NULL;
    
    return ret;
}

static int _test_load(void){
    return 0;

    uint32_t hv = 0;
    char *key = "load_key";
    char *data = "load data";
    int nkey = strlen(key);
    int nbytes = strlen(data) ;
    time_t realtime1 = 60000;
    item *it = item_alloc(key, nkey, 0, realtime(realtime1), nbytes + 2);
	if( NULL == it )
        return ERR_FAIL;

    memcpy(ITEM_data(it), data, nbytes);
    memcpy(ITEM_data(it) + nbytes, "\r\n", 2);

    hv = hash(ITEM_key(it), it->nkey);
    item_lock(hv);
    do_item_link(it,hv);
    item_unlock(hv);

    char *key2 = "load_key2";
    char *data2 = "load data2";
    nkey = strlen(key2);
    nbytes = strlen(data2) ;
  
    item *it2 = item_alloc(key2, nkey, 0, realtime(realtime1), nbytes + 2);
	if( NULL == it2 )
        return ERR_FAIL;

    memcpy(ITEM_data(it2), data2, nbytes);
    memcpy(ITEM_data(it2) + nbytes, "\r\n", 2);

   
   
    hv = hash(ITEM_key(it2), it2->nkey);
    item_lock(hv);
    do_item_link(it2,hv);
    item_unlock(hv);
    
   
	return ERR_OK;
}

static int _load_each_item( TDB_CONTEXT *tdb, 
                            TDB_DATA key, 
                            TDB_DATA dbuf, 
                            void *state){
   // return 0;
    static int ref = 0;
    uint32_t hv = 0;
    KEYDATA keydata ;
    
    to_keydata((void*)dbuf.dptr, &keydata);
    
    item *it = item_alloc((char*)key.dptr, 
                           key.dsize - 1, 
                           keydata.flag, 
                           keydata.exptime, 
                           keydata.nbytes );
	if( NULL == it )
        return ERR_FAIL;

    memcpy(ITEM_data(it), keydata.data, keydata.nbytes );
    memcpy(ITEM_data(it) + keydata.nbytes, "\r\n", 2);
    it->refcount = 1;
    
    hv = hash(ITEM_key(it), it->nkey);
    item_lock(hv);
    do_item_link(it,hv);
    item_unlock(hv);

    printf("[%u]load [%s] finish [ref:%d]\n",(unsigned)time(NULL), key.dptr, ref++);
	return ERR_OK;
}


static int _mkv_load(DATABASE *db){
     
    if( NULL == db )
        return ERR_DB_INVALID;
    _test_load();
    //printf("[%u]load starts\r",(unsigned)time(NULL));
    dump_tdb(db, _load_each_item);
 
	return ERR_OK;
}

int mkv_new_db(const char *dbname){
    int ret = ERR_OK;

    ret = create_db( dbname );
    if( ERR_OK != ret )
        return ret;

    return ERR_OK;
}

int mkv_del_key(const unsigned char *key, DATABASE *db){
    return delete_key(key, db);
    return ERR_OK;
}

int mkv_close_db(DATABASE *db){
    int ret = ERR_OK;

    ret = close_db( db );
    if( ERR_OK != ret )
        return ret;

    return ret;
}

DATABASE *mkv_open_db(const char *dbname){
    
    DATABASE *db = open_db( dbname );
    if( NULL == db )
        return NULL;

    return db;
}

int mkv_set_key( const unsigned char *key, 
            const unsigned char *value, 
            int size, 
            DATABASE *db){
    int ret = ERR_OK;

    ret = set_key(key, value, size, db );
    if( ERR_OK != ret )
        return ret;
    
    return ret;
}

int mkv_get_val(  const unsigned char *key, 
                unsigned char **value, 
                int *size, 
                DATABASE *db ){               
    int ret = ERR_OK;

    ret = get_value(key, value, size, db );
    if( ERR_OK != ret )
        return ret;
    
    return ret;
}

int mkv_fetch_keys(void){
    return 0;
}
/**@brief mkv data initialize

*/
DATABASE* mkv_init( const char *dbname ){ 
    DATABASE *db = NULL;

    if( ERR_OK != _block_init() ){
        return db;
    }

    db = open_db( dbname );
    if( db ) {
        /*database is valid & eixsts*/
        _mkv_load(db);
		return db;
	}

    if( ERR_OK != mkv_new_db( dbname ) ){
        return db;
    }

    db = open_db( dbname );

    return db ;
}

int to_string(  int flag,               //[in]flag
                int nbytes,             //[in]size of data
                int nkey,               //[in]size of key
                unsigned int exptime,   //[in]expire time
                char *data,             //[in]data
                int  *size,             //[out]the length of all data
                unsigned char **pFinal){
    int curr = 0;
    *size = sizeof(int)*2 + sizeof(unsigned int) + nbytes + 2;

    if( *size <= MAX_CHUNK ){
         memset(g_buf, 0x0, MAX_CHUNK);
        *pFinal = g_buf;
       
    }else{
        *pFinal = calloc(sizeof(char), *size + 1);
        if( NULL == *pFinal ){
            return ERR_STRING_NULL;
        }
    }
    
    memmove(*pFinal, &flag, sizeof(int));
    curr += sizeof(int);
    
    memmove(*pFinal + curr, &nbytes, sizeof(int));
    curr += sizeof(int);
    
    memmove(*pFinal + curr, &nkey, sizeof(int));
    curr += sizeof(int);
    
    memmove(*pFinal + curr, &exptime, sizeof(unsigned int));
    curr += sizeof(int);
    
    memmove(*pFinal + curr, data, nbytes);
    
    return ERR_OK;
}

int to_keydata(unsigned char *data, KEYDATA *keydata){
    int curr = 0;
    if( NULL == data || NULL == keydata )
        return ERR_STRING_NULL;
   
    memmove( (void*)&keydata->flag, (void*)(data + curr), sizeof(int));
    curr += sizeof(int);
    
    memmove( (void*)&keydata->nbytes, (void*)(data + curr), sizeof(int));
    curr += sizeof(int);
    
    memmove( (void*)&keydata->nkey, (void*)(data + curr), sizeof(int));
    curr += sizeof(int);
    
    memmove( (void*)&keydata->exptime, (void*)(data + curr), sizeof(unsigned int));
    curr += sizeof(unsigned int);
    
    keydata->data = data + curr;
    
    return ERR_OK;
}
