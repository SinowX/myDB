#ifndef __DB_STRUCT
#define __DB_STRUCT

#include<sys/types.h>
// #include<stdlib.h>
#include<stdint.h>
#include"db_limits.h"


#ifndef __VAL_UNION
#define __VAL_UNION
union val_union{
        int i_val;
        double d_val;
        char c_val[ROW_VALUE_SIZE];
        time_t t_val;
};
#endif


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



typedef struct table_meta{
    time_t id;
    char name[TABLE_NAME_SIZE];
    off_t index_info_offset;
    off_t data_info_offset;
}table_meta;


//索引表信息
typedef struct index_table_info{
    off_t index_offset;
    uint8_t clause_num;
    // off_t index_size;
    //index_item 在添加
    // __index_clause__ clause_array[MAX_INDEX_TABLE];  
} index_table_info;



//数据表信息
typedef struct data_table_info{
    // off_t data_size;
    off_t data_offset;
    uint clause_num;
    // __data_clause__ clause_array[MAX_COLUMN];
} data_table_info;

//此表的索引表存储块信息
typedef struct index_table{
    // 索引表数量
    uint8_t item_count;
    // 所有索引表的信息的数组
    index_item_info item_info[MAX_INDEX_TABLE];
    //当前此索引表中 storage space 的数量，最大是MAX_INDEX_STORAGE_TABLE
    uint8_t storage_num;
    //此索引表中 storage space 数组
    __storage_info__ storage_table[MAX_INDEX_STORAGE_TABLE];

    // __index_node__ *index_header; // 注意初始化索引 b+ 树
}index_table;

typedef struct data_table{
    uint8_t col_num;
    col_item col_itm[MAX_COLUMN];
    uint8_t storage_num;
    __storage_info__ storage_table[MAX_DATA_STORAGE_TABLE];
    // void *data_field;
}data_table;







// 索引表节点结构
typedef struct index_node{
    uint8_t range_left;
    uint8_t range_middle;
    uint8_t range_right;
    uint8_t range_end; 
    // range_left <= * < range_middle <= * < range_right <= * < range_end
    // 叶子节点 即为 一个字母对应一个区 left==middle==right==end
    // 叶子节点 lchild mchild rchild 置空
    uint8_t lchild;
    uint8_t mchild;
    uint8_t rchild;

} index_node;


//根据 value 定位对应的 row
typedef struct value2data{
    val_union value;
    off_t offset; // offset to data
} __value2data__;


//根据 value 定位 row 的 数组
typedef struct value2data_table{
    // 当前 value2data 中的数据数量，防止访问越界
    uint8_t count;
    __value2data__ value2data[MAX_VALUE2DATA_TABLE];
}value2data_table;



//索引条目 包括 B+ 树 以及 保存 value-data 查找表
typedef struct index_item{
    char column_name[COLUMN_NAME_SIZE];
    uint8_t flags;
    // 为树的每一个叶子结点开辟一个table
    value2data_table table[TRIBLE_TREE_LEAF_LENGTH];
    index_node tree[TRIBLE_TREE_LENGTH];
    // off_t index_item_size;
    // char item_space[MAX_INDEX_ITEM_SIZE];
}index_item;


typedef struct index_item_info{
    char column_name[COLUMN_NAME_SIZE];
    // index_item 的偏移量
    off_t offset;
}index_item_info;


// 为了解决创建表时 列的数量、；类型不确定带来的问题，因而对数据段创建 内存管理系统

//列的相关属性
typedef struct col_item{
    char col_name[COLUMN_NAME_SIZE];
    uint8_t flags;
}col_item;

//每行的值为一个offset链表，从第一个col开始，到最后一个col
typedef struct row_item{
    uint8_t flags;
    val_union value;
    // off_t next;
}row_item;




//通用 storage space info 模型
typedef struct storage{
    // storage space 下标
    uint8_t index;
    off_t offset;
    off_t cur_offset;
}__storage_info__;

//index storage space
typedef struct storage_index{
    char data[MAX_INDEX_STORAGE_SIZE];
}storage_index;

//data storage space
typedef struct storage_data{
    char data[MAX_DATA_STORAGE_SIZE];
}storage_data;

#endif