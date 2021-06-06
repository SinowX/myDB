#include<stdio.h>
#include<stdlib.h>
#include"db_limits.h"

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

enum ATTR{
    INT,
    DOUBLE,
    STRING,
    TIME,
    AUTOINC,
    PRIMARY,
    INDEX
};


typedef struct CreateAttr{
    char name[COLUMN_NAME_SIZE];
    char attr[COLUMN_ATTR_SIZE];
    struct CreateAttr * next;
}CreateAttr;

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
    char second[MAX_VALUE_SIZE];

    struct Condi * next;
    
}Condi;

typedef Condi SelectCondi;
typedef Condi UpdataCondi;
typedef Condi DropRowCondi;


typedef struct InsertColVal{
    //列名
    char name[COLUMN_NAME_SIZE];
    //列值
    char value[MAX_VALUE_SIZE];

    struct InsertColVal * next;
}InsertColVal;