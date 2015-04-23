#include "errdesc.h"
#include <stdio.h>
const char* g_errdesc[] = {
        "ok",                                            /*0*/
        "fail",                                          /*1*/
        "create tdb failed",                             /*2*/
        "string is null",                                /*3*/
        "database is duplication",                       /*4*/
        "database initailize failed",                    /*5*/
        "database create failed",                        /*6*/
        "database store data failed",                    /*7*/
        "database open failed",                          /*8*/
        "database invalid",                              /*9*/
        "create new thread failed",                      /*10*/
        "create new pipe failed",                        /*11*/
        "initialize base event failed",                  /*12*/
        "pointer is NULL"                                /*13*/
    };

void displayErr( int errno )
{
    printf("%s\n" , g_errdesc[errno]);
    return ;//g_errdesc[errno];
}

