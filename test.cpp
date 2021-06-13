#include<stdio.h>
#include<iostream>
#include"./header/rt_class.h"
#include<string.h>


int main()
{
    DBMGR myMgr;
    char tbname[]="shabi";
    // CreateAttr test;
    // test.flags=ATTR::DOUBLE|ATTR::PRIMARY;
    // strcpy(test.name,"rthoi");
    // // test.next=NULL;

    // CreateAttr test2;
    // test2.flags=ATTR::INT;
    // strcpy(test2.name,"kpofd");
    // test2.next=NULL;

    // test.next=&test2;

    // myMgr.CreateTable(tbname,&test);
    // uint8_t flag=ATTR::INT|ATTR::PRIMARY;



    // uint8_t sad=flag&ATTR::DOUBLE;
    
    // myMgr.ShowIndex(tbname);
    
    InsertColVal the_insert,the_insert2;

    strcpy(the_insert.name,"rthoi");
    the_insert.flags=ATTR::DOUBLE|ATTR::PRIMARY;
    the_insert.value.i_val=123;
    
    strcpy(the_insert2.name,"kpofd");
    the_insert2.flags=ATTR::INT;
    the_insert2.value.d_val=double(23.12);

    the_insert.next=&the_insert2;
    the_insert2.next=NULL;

    



    myMgr.Insert(tbname,&the_insert);



    // char tbname2[]="sinow";
    // myMgr.ShowIndex(tbname2);
    // myMgr.ShowTables();
    // myMgr.ShowColumns(tbname);
    // DBMGR another;
    // another.CreateTable(tbname,&test);
}