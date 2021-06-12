#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include<string>
#include<stack>
#include<queue>
#include<math.h>

#include"db_struct.h"
#include"sql_struct.h"
#include"ERROR.h"
#include"utils.h"


#define DB_PATH "./DBFILE"


class DBMGR{
    private:
        //DBFile 文件描述符
        int fd;
        //当前DBFile的结尾偏移量
        off_t cur_offset_end;
        //DB File 文件头
        DB_Header db_header;

        //避免频繁使用 malloc free
        table_meta tb_meta;
        index_table_info idx_tb_info;
        data_table_info data_tb_info;
        index_table idx_tb;
        data_table data_tb;

        off_t storage_begin;


        //如果 构造函数打开的DBFile为空，则调用该函数来初始化表结构
        void InitDBFile();
        void InitIndex(uint8_t col_idx,index_item * idx_itm);
        void Distribute(uint8_t hashed,index_node * tree_node, off_t offset);

        void LoadInfo(char * tbname);

        value2data_table LocateWithIndex(index_item idx_itm, val_union value);

        void GetTribleTree(index_node *tree);

    public:
        //打开DBFile，初始化 fd,db_header
        DBMGR();
        //创建表
        int CreateTable(char * tbname,CreateAttr *attr);
        //查找
        char * Select(char * tbname,SelectCol col,SelectCondi *condi);
        //展示数据库中所有表名
        char * ShowTables();
        //展示某个表的列信息
        char * ShowColumns(char * tbname);
        //展示某个表的统计信息
        char * ShowIndex(char * tbname);
        //增填条目
        int Insert(char * tbname,InsertColVal *col_val);
        //修改条目
        int Update(char * tbname,char *colname,val_union colvalue,UpdataCondi *condi);
        //删除表
        int DropTable(char * tbname);
        //删除行
        int DropRow(char * tbname,DropRowCondi *condi);
};
