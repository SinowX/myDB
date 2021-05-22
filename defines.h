// #include<time.h>
#include<sys/types.h>
// #include<stdlib.h>
#include<stdint.h>


#define MAX_TABLE_BLOCK 2<<5 // 文件头记录的数据表的最大数量
#define TABLE_CLAUSE_SIZE NULL //文件头 每个条目信息 大小
// typedef time_t tableid_t 
// typedef __off_t 
#define MAX_INDEX_TABLE 64 // 一个数据表对应的索引表 最大数量
#define MAX_DATA_TABLE 64  // 数据表 最大数量


// 数据存储时 value 的 首字母 应为 a-z
// 26
// 8 9 9
// 2 3 3 3 3 3 3 3 3
// 



// table_block start
    
    // meta_info start

        // index_table_info start

#define COLUMN_NAME_SIZE 128 // 列名 大小限制
#define COLUMN_TYPE_SIZE 16 // 列类型 大小限制


typedef struct index_clause{
    char  column_name[COLUMN_NAME_SIZE];
    char column_type[COLUMN_TYPE_SIZE];
    off_t index_offset;
    uint8_t index_size;
} __index_clause__;



typedef struct index_table_info{
    uint clause_num;
    __index_clause__ clause_array[MAX_INDEX_TABLE];  
} index_table_info;

        // index_table_info end

        // data_table_info start

#define MAX_COLUMN 64 // 每个数据表最大列数

typedef struct data_clause{
    char column_name[COLUMN_NAME_SIZE];
    char column_type[COLUMN_TYPE_SIZE];
    uint8_t data_size; // 数据大小应在选定 column type 后进行确定
    bool if_null;
    bool if_primary;
    bool if_auto_increment;
}__data_clause__;


typedef struct data_table_info{
    off_t data_size;
    off_t data_offset;
    uint clause_num;
    __data_clause__ clause_array[MAX_COLUMN];
} data_table_info;

        // data_table_info end
    
    // mata_info end

    // index_table_block start

// 索引表节点结构
typedef struct index_node{
    uint8_t range_left;
    uint8_t range_middle;
    uint8_t range_right;
    uint8_t range_end; 
    // range_left <= * < range_middle <= * < range_right <= * < range_end
    // 叶子节点 即为 一个字母对应一个区 left==middle==right==end
    // 叶子节点 lchild mchild rchild 置空
    index_node* lchild;
    index_node* mchild;
    index_node* rchild;

    value2data_table* table;    // 非叶子节点置空 

} __index_node__;


#define MAX_VALUE2DATA_TABLE 2<<5
#define MAX_VALUE_SIZE 2<<7
typedef struct value2data{
    char value[MAX_VALUE_SIZE];
    off_t offset; // offset to data
} __value2data__;

typedef struct value2data_table{
    __value2data__ value2data[MAX_VALUE2DATA_TABLE];
}value2data_table;


#define MAX_INDEX_STORAGE_TABLE 2<<2
#define MAX_INDEX_STORAGE_SIZE 2<<19 // 每个存储区的大小未确定

typedef struct storage{
    uint8_t num;
    off_t offset;
}__storage__;



typedef struct index_table{
    __storage__ storage_table[MAX_INDEX_STORAGE_TABLE];
    __index_node__ *index_header; // 注意初始化索引 b+ 树
}index_table;

    // index_table_block end

    // data_table_block start

#define MAX_DATA_STORAGE_TABLE 2<<2
#define MAX_DATA_STORAGE_SIZE 2<<19 // 未确定


// 为了解决创建表时 列的数量、；类型不确定带来的问题，因而对数据段创建 内存管理系统

typedef struct data_table{
    __storage__ storage_table[MAX_DATA_STORAGE_TABLE];
    void *data_field;
}data_table;
