#include "db_serv.h"
#include "memcached.h"
#include "mkvdb.h"
#include <strings.h>

#if 0
static int db_cli;
int db_serv_setup(){
   struct sockaddr_in servaddr;
   db_cli = socket(AF_INET,SOCK_STREAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
   servaddr.sin_port=htons(settings.port);

   connect(db_cli, (struct sockaddr *)&servaddr, sizeof(servaddr));

    return ERR_OK;
}
#endif