#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include"db_struct.h"
#include"sql_struct.h"
#include"ERROR.h"
#include"utils.h"


#define DB_PATH "./DBFILE"


class DBMGR{
    private:
        //DBFile 文件描述符
        int fd;
        //DB File 文件头
        DB_Header *db_header;

        //如果 构造函数打开的DBFile为空，则调用该函数来初始化表结构
        void InitDBFile();
        

    public:
        //打开DBFile，初始化 fd,db_header
        DBMGR();
        //创建表
        int CreateTable(char * tbname,CreateAttr *attr);
        //查找
        char * Select(char * tbname,SelectCondi *condi);
        //展示数据库中所有表名
        char * ShowTables();
        //展示某个表的列信息
        char * ShowColumns(char * tbname);
        //展示某个表的统计信息
        char * ShowIndex(char * tbname);
        //增填条目
        int Insert(char * tbname,InsertColVal *col_val);
        //修改条目
        int Update(char * tbname,UpdataCondi *condi);
        //删除表
        int DropTable(char * tbname);
        //删除行
        int DropRow(char * tbname,DropRowCondi *condi);
};

// public start

DBMGR::DBMGR(){
    int fd;
    if((fd=open(DB_PATH,O_RDWR|O_CREAT,0644))==-1)
        pError();
    
    this->fd=fd;
    //获取当前偏移量
    off_t cur_seek=lseek(this->fd,0,SEEK_END);

    if(cur_seek==-1)
        pError();
    else if(cur_seek==0)
        //DBFile 未初始化
        this->InitDBFile();
    //重置偏移量到0
    if(lseek(this->fd,0,SEEK_SET)==-1)
        pError();
    this->db_header=(DB_Header*)malloc(sizeof(DB_Header));
    if(write(this->fd,this->db_header,sizeof(DB_Header))!=sizeof(DB_Header))
        pError("Init this.db_header ERROR");

}

//不允许建立空表，每个列必须指定其类型，不可为空
int DBMGR::CreateTable(char * tbname,CreateAttr *attr){
    while(attr->next!=NULL)
    {

    }

}
char * DBMGR::Select(char * tbname,SelectCondi *condi){

}
char * DBMGR::ShowTables(){

}
char * DBMGR::ShowColumns(char * tbname){

}
char * DBMGR::ShowIndex(char * tbname){

}
int DBMGR::Insert(char * tbname,InsertColVal *col_val){

}
int DBMGR::Update(char * tbname,UpdataCondi *condi){

}
int DBMGR::DropTable(char * tbname){

}
int DBMGR::DropRow(char * tbname,DropRowCondi *condi){

}

// public end

// private start

void DBMGR::InitDBFile()
{
    DB_Header *header=(DB_Header*)malloc(sizeof(DB_Header));
    header->table_num=0;
    ssize_t numwrite=write(this->fd,header,sizeof(DB_Header));
    if(numwrite==-1)
        pError();
    else if(numwrite!=sizeof(DB_Header))
        pError("Write Wrong");
}



// private end
