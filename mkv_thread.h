#ifndef _MKV_THREAD_H
#define _MKV_THREAD_H

int mkvFds[2];/*pipe for IO*/




void *_mkv_start_thread(void *args);

int _mkv_setup_thread(void *args);
//void _mkv_thread_process(int fd, short which, void *args);
void _mkv_process_queue(int fd, short which, void *args);
int _read_handle( int fd, char **pBuff, int *size );
int _write_handle( int fd, char *pBuff, int size );
int mkv_thread_init(void);
int read_pipe( char *pBuff );
int write_pipe( char *pBuff, int size );



#endif