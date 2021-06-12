#include<stdio.h>
#include<iostream>
#include"rt_class.h"

int main()
{
    DBMGR myMgr;
    char tbname[]="sinow";
    CreateAttr test;
    test.flags=ATTR::INT|ATTR::PRIMARY;
    strcpy(test.name,"qwe");
    test.next=NULL;


    myMgr.CreateTable(tbname,&test);
}