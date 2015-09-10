#include "msg_queue.h"
#include <stdlib.h>
#include<pthread.h>

static pthread_mutex_t mutex;
static QLItem *g_header;
static QLItem *g_tail;
static QLItem *g_curr;

int queue_init(){

    if(0 != pthread_mutex_init(&mutex, NULL) ){
        return -1;
    }
    
    g_header = (QLItem*)malloc(sizeof(QLItem));
    if( NULL == g_header )
        return -2;

    g_header->next = NULL;
    g_tail = g_curr = g_header; 
    
    return 0;
}

int queue_push(item *item, enum process_type action){
    printf("queue_push starts\n");
    QLItem* tagItem = (QLItem*)malloc(sizeof(QLItem));
    if( NULL == tagItem)
        return -1;

    tagItem->item = item;
    tagItem->action = action;
    tagItem->next = NULL;

    pthread_mutex_lock(&mutex);
    g_tail->next = tagItem;
    g_tail = tagItem;
    pthread_mutex_unlock(&mutex);
    printf("queue_push ends\n");
    return 0;
}

int queue_front(item *item, enum process_type action){
    QLItem* tagItem = (QLItem*)malloc(sizeof(QLItem));
    if( NULL == tagItem)
        return -1;

    tagItem->item = item;
    tagItem->action = action;
    
    pthread_mutex_lock(&mutex);
    tagItem->next = g_header->next;
    g_header->next = tagItem;
    pthread_mutex_unlock(&mutex);
    
    return 0;
}

QLItem* queue_pop(){
    printf("pop starts\n");
    pthread_mutex_lock(&mutex);
    if( NULL == g_header->next ){
        g_tail = g_header;
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    g_curr = g_header->next;
    g_header->next = g_curr->next;
    pthread_mutex_unlock(&mutex);
    printf("pop ends\n");
    return g_curr;
}
