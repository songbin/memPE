#include "dbopt.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "utility.h"
#include <ctype.h>


int create_db( const char *DBName ){
    int ret = ERR_OK;
    if( NULL == DBName )
        return ERR_STRING_NULL;

    if( 0 == access( DBName , 0 ) )
    {
        unlink(DBName);
       // return ERR_DB_DUPLICATE;
    }

    make_dir_needed( DBName );
    struct tdb_context *tdb = tdb_open(DBName, 0, 0, O_CREAT | O_TRUNC | O_RDWR, 0600);
	if (!tdb)
		return ERR_DB_NEW_FAIL;

	char *value = "1.0";
    char *key = "version";
    int size = strlen(value) + 1;

    TDB_DATA dkey, data;
	dkey.dptr = (unsigned char *) key;
	dkey.dsize = strlen(key) + 1;
	data.dptr = (unsigned char *) value;
	data.dsize = size;

	if (tdb_store(tdb, dkey, data, 0))
		ret = ERR_DB_STORE_FAIL;

    tdb_close(tdb);

    return ret;
}

DATABASE* open_db( const char *DBName ){
    
    if( NULL == DBName )
        return NULL;

    struct tdb_context *tdb = tdb_open(DBName, 0, 0, O_RDWR | O_CREAT, 0666);
    if( NULL == tdb )
        return NULL;

    DATABASE *db = (DATABASE*)malloc(sizeof(DATABASE));
    db->tdb = tdb;
    sem_init( &db->sem, 0, 1 );
    db->stamp = time(NULL);
   // printf("time is %u\n", db->stamp );
    
    return db;
}

int close_db( DATABASE *db )
{
    int ret = ERR_OK;
    
    if( NULL == db )
        return ERR_DB_INVALID;

    tdb_close(db->tdb);
    sem_close(&db->sem);
    
    return ret;
}
/**
 * Returns a single value from the database
 *
 * @warning The caller must free the returned data.
 *
 * @param [out]	   value	pointer to the value being read
 * @param [out]    size	    number of bytes of @c value parameter
 * @param [in]     key		pointer to a string containing the key in the ENV db
 * @param [in]     db		pointer of a tdb object;
 *
 * @return error code
 * 	@retval ERR_OK = OK
 * 	@retval ERR_DB_INVALID = failed to access tdb
 * 	@retval ERR_STRING_NULL = key not found
 */
int get_value(const unsigned char *key, 
             unsigned char **value, 
             int *size, 
             DATABASE *db){
             
    int ret = ERR_OK;
    TDB_DATA dkey,dvalue ;

    if( NULL == db )
        return ERR_DB_INVALID;
    
    dkey = string_to_tdb(key);
    if( 0 == dkey.dsize )
        return ERR_STRING_NULL;

    while( sem_wait( &db->sem ) == -1 && errno == EINTR );
	dvalue = tdb_fetch(db->tdb, dkey);
	sem_post( &db->sem );

	if (NULL == dvalue.dptr)
		ret = ERR_STRING_NULL;

	*value =  dvalue.dptr;
	*size = dvalue.dsize;

	free(dkey.dptr);
    return ret;
}

int delete_key(const unsigned char *key, DATABASE *db){
	if( NULL == db || NULL == db->tdb)
        return ERR_DB_INVALID;

	TDB_DATA dkey;
	dkey = string_to_tdb(key);

    while( sem_wait(&db->sem) == -1 && errno == EINTR );
	int err = tdb_delete(db->tdb, dkey);
	free(dkey.dptr);
	sem_post(&db->sem);

	if (err)
		return ERR_FAIL;
    
	return 0;
}
/**
 * Returns the key structure of the database properly filled
 *
 * @warning The caller must free the returned data.
 *
 * @param [in] key	string containing the value of the key
 * @return @c TBD_DATA struct ready to be used
 */ 
TDB_DATA string_to_tdb(const unsigned char *key){

    TDB_DATA data;
    data.dptr = NULL;
    data.dsize = 0;
    if( NULL == key )
        return data;
    
    int size = strlen( (char*)key ) + 1;
    data.dptr = (unsigned char *)calloc( size , sizeof(unsigned char));
    memcpy( data.dptr, key, size );
    data.dsize = size;
    
    return data;
}
/**
 * Writes a single value into the database
 *
 * @param [in]     key		pointer to a string containing the key in the ENV db
 * @param [in]     value	string to be written into the ENV db
 * @param [in]     size		length of @c value (including trailing '\0')
 * @param [in,out] db		pointer of an open tdb object
 *
 * @return error code
 * 	@retval 0 = OK
 * 	@retval 1 = cannot write data into database
 */
int set_key( const unsigned char *key, 
            const unsigned char *value, 
            int size, 
            DATABASE *db){
    int ret = ERR_OK;
    if(NULL == db || NULL == key || NULL == value )
        return ERR_STRING_NULL;
    
	TDB_DATA dkey, data;
	dkey.dptr = (unsigned char *) key;
	dkey.dsize = strlen((char*)key) + 1;
	data.dptr = (unsigned char *) value;
	data.dsize = size;

    while( sem_wait( &db->sem ) == -1 && errno == EINTR );
	if (tdb_store(db->tdb, dkey, data, 0))
		ret = ERR_DB_STORE_FAIL;
	sem_post( &db->sem );


    return ret;
}

int dump_tdb(DATABASE *db, tdb_traverse_func fn)
{
	if (!db || !db->tdb) {
		return ERR_DB_INVALID;
	}
    
    tdb_traverse(db->tdb, fn, NULL);
	
	return ERR_OK;
}