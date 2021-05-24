#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

int main(int argc, char *argv[])
{
    int fd;

    // if((fd = open(argv[1], O_RDWR | O_CREAT,755)) == -1){
    //     printf("%d\n", errno);     
    //     perror("open");            
    //     exit(1);                   
    // }                              

    fd=open("./tmpDB",O_RDWR|O_CREAT,0644);
    if(fd==-1)
    {
        printf("Error: %s\n",strerror(errno));
        exit(-1);
    }
    // printf("%d\n", fd);

    close(fd);
    return 0;
}