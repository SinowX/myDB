// #include"ERROR.h"
#include"./header/ERROR.h"
#include<stdio.h>
#include<string.h>
#include<errno.h>
// #include<string>
#include<iostream>
#include<stdlib.h>

void pError()
{
    printf("ERROR: %s\n",strerror(errno));
    exit(-1);
}

void pError(std::string msg)
{
    // printf("ERROR: %s\n",msg);
    std::cout<<"ERROR: "<<msg<<std::endl;
    exit(-1);
}

void pWarn()
{
    printf("WARN: %s\n",strerror(errno));
}

void pWarn(std::string msg)
{
    std::cout<<"WARN: "<<msg<<std::endl;
    // printf("WARN: %s\n",msg);
}