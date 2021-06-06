// #include<time.h>
#include<sys/types.h>
// #include<stdlib.h>
#include<stdint.h>
#include"db_limits.h"


// DB_Header start

//文件头保存的 table 条目信息
typedef struct table_clause{
    //表名
    char name[TABLE_NAME_SIZE];
    //表块条目唯一识别id
    time_t id;
    //表块元信息存储位置
    off_t offset;
    //索引表数量
    uint8_t index_num;
    //数据表数量
    uint8_t data_num;
}__table_clause__;



//文件头 信息
typedef struct DB_Header{
    uint8_t table_num;

    //table条目信息数组
    __table_clause__ clause_array[MAX_TABLE_BLOCK];
}DB_Header;


// DB_Header end


// 数据存储时 value 的 首字母 应为 a-z
// 26
// 8 9 9
// 2 3 3 3 3 3 3 3 3
// 



// table_block start
    //table_meta start
typedef struct table_meta{
    time_t id;
    char name[TABLE_NAME_SIZE];
    off_t index_info_offset;
    off_t data_info_offset;
}table_meta;

    //table meta end

    // meta_info start

        // index_table_info start



//索引表信息条目
typedef struct index_clause{
    char  column_name[COLUMN_NAME_SIZE];
    char column_type[COLUMN_TYPE_SIZE];

    //指向该索引表条目 index_item 的地址
    off_t index_offset;
    uint8_t index_size;
} __index_clause__;


//索引表信息
typedef struct index_table_info{
    uint clause_num;
    __index_clause__ clause_array[MAX_INDEX_TABLE];  
} index_table_info;

        // index_table_info end

        // data_table_info start


//数据表信息条目
typedef struct data_clause{
    char column_name[COLUMN_NAME_SIZE];
    char column_type[COLUMN_TYPE_SIZE];
    uint8_t data_size; // 数据大小应在选定 column type 后进行确定
    bool if_null;
    bool if_primary;
    bool if_auto_increment;
}__data_clause__;

//数据表信息
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


//根据 value 定位对应的 row
typedef struct value2data{
    char value[MAX_VALUE_SIZE];
    off_t offset; // offset to data
} __value2data__;


//根据 value 定位 row 的 数组
typedef struct value2data_table{
    __value2data__ value2data[MAX_VALUE2DATA_TABLE];
}value2data_table;

//通用 storage space 模型
typedef struct storage{
    // storage space 下标
    uint8_t index;
    off_t offset;
}__storage__;


//索引条目
typedef struct index_item{
    char column_name[COLUMN_NAME_SIZE];
    char column_type[COLUMN_TYPE_SIZE];
    off_t index_item_size;
    char item_space[MAX_INDEX_ITEM_SIZE];
}index_item;


//此表的索引表存储块信息
typedef struct index_table{
    //当前此索引表中 storage space 的数量，最大是MAX_INDEX_STORAGE_TABLE
    uint8_t storage_num;
    //此索引表中 storage space 数组
    __storage__ storage_table[MAX_INDEX_STORAGE_TABLE];

    // __index_node__ *index_header; // 注意初始化索引 b+ 树
}index_table;

    // index_table_block end

    // data_table_block start


// 为了解决创建表时 列的数量、；类型不确定带来的问题，因而对数据段创建 内存管理系统

typedef struct data_table{
    uint8_t storage_num;
    __storage__ storage_table[MAX_DATA_STORAGE_TABLE];
    void *data_field;
}data_table;
