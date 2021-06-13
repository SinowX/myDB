#ifndef __SQL_STRUCT
#define __SQL_STRUCT

#include<stdio.h>
#include<stdlib.h>
#include"db_limits.h"

const double EPS = 1e-6;


#ifndef __VAL_UNION
#define __VAL_UNION
union val_union{
        int i_val;
        double d_val;
        char c_val[ROW_VALUE_SIZE];
        time_t t_val;
};
#endif

enum OPT{
    NEQUAL,
    LESS,
    LEQUAL,
    EQUAL,
    GEUQAL,
    GREATER
};

//type
const char ATTR_INT[]="int";
const char ATTR_DOUBLE[]="double";
const char ATTR_STRING[]="string";
const char ATTR_TIME[]="time";

//自增
const char ATTR_AUTOINC[]="auto_increment";
//主键 默认有索引
const char ATTR_PRIMARY[]="primary";
//添加索引
const char ATTR_INDEX[]="index";

//flags 掩码
namespace ATTR{
    //int32_t
    const int8_t INT=01;
    //double
    const int8_t DOUBLE=02;
    //char []
    const int8_t STRING=04;
    //time_t
    const int8_t TIME=010;
    
    const int8_t AUTOINC=020;
    const int8_t PRIMARY=040;
    const int8_t INDEX=0100;
};


typedef struct CreateAttr{
    char name[COLUMN_NAME_SIZE];
    uint8_t flags;
    struct CreateAttr * next;
}CreateAttr;


typedef struct InsertColVal{
    //列名
    char name[COLUMN_NAME_SIZE];
    //列值
    val_union value;
    //属性值
    uint8_t flags;

    struct InsertColVal * next;
}InsertColVal;


// typedef struct SelectStruct{
//     SelectCol *column;
//     SelectCondi condition;
// }SelectStruct;


typedef struct SelectCol{
    char name[COLUMN_NAME_SIZE];
    struct SelectCol * next;
}SelectCol;

//通用 Condition 结构
typedef struct Condi{
    // 运算符，查看 OPT enum
    int opt;
    // 第一个参数，应为 列名
    char first[COLUMN_NAME_SIZE];
    // 第二个参数，应为 值，其类型由 first 的 列 type 确定
    val_union second;

    struct Condi * next;

}Condi;


// condi 至少要有一个结点
typedef Condi SelectCondi;
typedef Condi UpdataCondi;
typedef Condi DropRowCondi;


#endif