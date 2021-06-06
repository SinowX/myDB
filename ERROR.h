#ifndef __ERROR
#define __ERROR



#include<stdio.h>
#include<string.h>
#include<errno.h>
// #include<unistd.h>
#include<stdlib.h>

void pError()
{
    printf("ERROR: %s\n",strerror(errno));
    exit(-1);
}

void pError(char * msg)
{
    printf("ERROR: %s\n",msg);
    exit(-1);
}

void pWarn()
{
    printf("WARN: %s\n",strerror(errno));
}

void pWarn(char * msg)
{
    printf("WARN: %s\n",msg);
}

#endif