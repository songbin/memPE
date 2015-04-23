#ifndef ERR_DESC_H
#define ERR_DESC_H

void displayErr( int errno );

#define ERR_OK                 0
#define ERR_FAIL               1
#define ERR_TDB_CREATE_FAIL    2
#define ERR_STRING_NULL        3
#define ERR_DB_DUPLICATE       4
#define ERR_DB_INIT_FAIL       5
#define ERR_DB_NEW_FAIL        6
#define ERR_DB_STORE_FAIL      7
#define ERR_DB_OPEN_FAIL       8
#define ERR_DB_INVALID         9
#define ERR_THREAD_FAILED      10
#define ERR_PIPE_FAIL          11
#define ERR_EVENT_FAILED       12
#define ERR_NULL               13

#endif