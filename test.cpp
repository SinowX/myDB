#include<stdlib.h>
#include"ERROR.h"
#include<sys/stat.h>
#include<fcntl.h>

int main()
{
    int fd=open("/hasodqw",O_RDONLY);
    if(fd=-1)
    {
        pError();
    }
}