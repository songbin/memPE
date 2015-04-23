#ifndef DB_OPT_H
#define DB_OPT_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tdb.h"
#include "errdesc.h"
#include <semaphore.h>

typedef struct tagDataBase{
    struct tdb_context *tdb;
    sem_t sem;
    unsigned int stamp;
}DATABASE;

int create_db(const char *DBName);
DATABASE* open_db(const char *DBName);
int close_db( DATABASE *db );

int set_key( const unsigned char *key, 
            const unsigned char *value, 
            int size, 
            DATABASE *db);
int get_value(const unsigned char *key, 
                unsigned char **value, 
                int *size, 
                DATABASE *db);
TDB_DATA string_to_tdb(const unsigned char *key);
int dump_tdb(DATABASE *db, tdb_traverse_func fn);
int delete_key(const unsigned char *key, DATABASE *db);

#endif