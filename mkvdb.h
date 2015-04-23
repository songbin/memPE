#ifndef MKV_DB_H
#define MKV_DB_H
#include "mkv_thread.h"
#include "dbopt.h"

#define MAX_CHUNK 1024*1024
#define FLAG_SAVE_ITEM 's'
#define FLAG_DEL_ITEM 'd'
#define FLAG_PRO_ITEM 'p'

typedef struct _key_data{
        int nkey;
        int nbytes;
        unsigned int exptime;
        int flag;
        unsigned char *data;
    }KEYDATA;

int mkv_new_db(const char *dbname);
DATABASE *mkv_open_db(const char *dbname);
int mkv_close_db( DATABASE *db );
int mkv_del_key(const unsigned char *key, DATABASE *db);

int mkv_set_key( const unsigned char *key, 
            const unsigned char *value, 
            int size, 
            DATABASE *db);
int mkv_get_val(const unsigned char *key, 
                unsigned char **value, 
                int *size, 
                DATABASE *db);
int mkv_fetch_keys(void);

DATABASE* mkv_init( const char *dbname );

int to_string(  int flag, 
                int nbytes, 
                int nkey,
                unsigned int exptime, 
                char *data, 
                int  *size,
                unsigned char **pFinal);
int to_keydata(unsigned char *data, KEYDATA *keydata);

#endif
