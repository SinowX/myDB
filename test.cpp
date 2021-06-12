#include<string>
#include<stack>
#include<stdio.h>
#include<iostream>
#include<string.h>
using namespace std;

union val_union{
        int i_val;
        double d_val;
        char c_val[10];
        time_t t_val;
};

int main()
{
    val_union *a,*b;
    a=(val_union *)malloc(sizeof(val_union));
    b=(val_union *)malloc(sizeof(val_union));
    
    memset(a,0,sizeof(val_union));
    memset(b,0,sizeof(val_union));

    if(!memcmp(a,b,sizeof(val_union)))
        printf("yes");
    else
        printf("no");


    return 0;    
}