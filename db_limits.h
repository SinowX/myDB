#ifndef __DB_LIMITS
#define __DB_LIMITS



// table name 存储大小
#define TABLE_NAME_SIZE 2<<5


#define MAX_TABLE_BLOCK 2<<5 // 文件头记录的数据表的最大数量
#define TABLE_CLAUSE_SIZE NULL //文件头 每个条目信息 大小
// typedef time_t tableid_t 
// typedef __off_t 
#define MAX_INDEX_TABLE 64 // 一个数据表对应的索引表 最大数量
#define MAX_DATA_TABLE 64  // 数据表 最大数量

//列名 大小限制
#define COLUMN_NAME_SIZE 128 
//列类型 大小限制
#define COLUMN_TYPE_SIZE 16 
//列属性 大小限制
#define COLUMN_ATTR_SIZE 128


#define MAX_COLUMN 64 // 每个数据表最大列数

//索引B+树的叶子结点可定位的最大 row 数量
//未确定
#define MAX_VALUE2DATA_TABLE 2<<7

//数据 value 的大小限制为 MAX_VALUE_SIZE*sizeof(char)
#define MAX_VALUE_SIZE 2<<7



//索引表存储空间最大数量
#define MAX_INDEX_STORAGE_TABLE 2<<2

//索引表存储空间大小
#define MAX_INDEX_STORAGE_SIZE 2<<19 //1MB



//索引条目的空间大小
#define MAX_INDEX_ITEM_SIZE 2<<15


#define MAX_DATA_STORAGE_TABLE 2<<2
#define MAX_DATA_STORAGE_SIZE 2<<21 // 4MB


#endif