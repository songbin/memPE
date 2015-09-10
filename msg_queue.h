#ifndef _MKV_MSG_QUEUE_H
#define _MKV_MSG_QUEUE_H

#include "memcached.h"

typedef struct _tagItem{
    item *item;
    enum process_type action;
    struct _tagItem *next;
}QLItem;

int queue_init(void);
int queue_push(item *item, enum process_type action);
int queue_front(item *item, enum process_type action);
QLItem* queue_pop(void);


#endif
