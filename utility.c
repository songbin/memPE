#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include <sys/stat.h> 
#include <unistd.h> 

void make_dir_needed(const char* szFileSpec)
{
#if 0
	if((char*)NULL == szFileSpec) return;
 	int len = strlen(szFileSpec);
    char buf[ 1024 ];

    if( len > 512 )
      return;

    strcpy( buf + len + 20, szFileSpec );
    char* szDir = dirname( buf + len + 20 );
    sprintf( buf, "mkdir -p %s", szDir );
    system( buf );
#else
	char strDir[256]={0};
    int len = strlen(szFileSpec);
	strncpy( strDir, szFileSpec, len );
	for( int nIndex = 0; nIndex < len; nIndex++ )
	{
		if( '/' == strDir[nIndex])
		{
			struct stat info;
			stat(szFileSpec,&info);
			if(!S_ISDIR(info.st_mode))
			{
				strDir[nIndex]=0;
				mkdir(strDir,0755);
				memset(strDir,0x00,256);
				strncpy(strDir,szFileSpec,len);
			}			
		}
	}
#endif
}