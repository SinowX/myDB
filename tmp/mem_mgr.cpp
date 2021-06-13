#include<iostream>
#include<stdio.h>
#include<stdlib.h>
// #include<sys/types.h>
#include<limits>
#include<string.h>

#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>


// using namespace std;

#define MAX_DATA_STORAGE_SIZE 2<<19
#define COLUMN_NAME_SIZE 128
#define MAX_COLUMN 64
#define MAX_DATA_STRING_SIZE 255

typedef struct column_struct{
    char column_name[COLUMN_NAME_SIZE]; //列名
    char type; //列类型
    uint8_t data_size; // 该列对应的数据大小
}column_struct;

typedef struct data_field{
    
    uint8_t column_num; // 列的总数量
    uint32_t column_size; // 列信息的总大小 MAX_COLUMN*sizeof(column_struct)
    
    column_struct column[MAX_COLUMN]; // 每一列的结构

    uint32_t row_size; // row 的大小 

    void * raw_mem; // 内存块开始地址 
    
    off_t raw_top; // 内存块的顶部 from 开始位置
    uint32_t raw_mem_size; // 内存块的总大小 MAX_DATA_STORAGE_SIZE

}data_field;


void initTable(data_field* mydata, uint8_t col_c, char col_n[][COLUMN_NAME_SIZE],char *col_t)
{

    if(col_c==0){
        perror("No Valid Columns\n");
        exit(-1);
    }

    for(int i=0;i<col_c;i++)
    {
        char given_type=col_t[i];
        switch (given_type)
        {
        case 'i':
            mydata->column_num++;
            mydata->column[i].type='i';
            mydata->column[i].data_size=sizeof(int32_t);
            strcpy(mydata->column[i].column_name,col_n[i]);
            break;
        case 'f':
            mydata->column_num++;
            // mydata->column_size+=sizeof(float);
            mydata->column[i].type='f';
            mydata->column[i].data_size=sizeof(float);
            strcpy(mydata->column[i].column_name,col_n[i]);
            break;
        case 'c':
            mydata->column_num++;
            // mydata->column_size+=MAX_DATA_STRING_SIZE;
            mydata->column[i].type='c';
            mydata->column[i].data_size=MAX_DATA_STRING_SIZE;
            strcpy(mydata->column[i].column_name,col_n[i]);
            break;
        default:
            printf("Invalid Column\n");
            exit(-1);
        }
    }
    mydata->column_size=mydata->column_num*sizeof(column_struct);
    mydata->raw_top+=mydata->column_size;
}

uint8_t getCol_info(char col_n[][COLUMN_NAME_SIZE],char *col_t)
{
    // printf("%x\n",col_t);
    int buffer=0;
    uint8_t count=0;
    while(1)
    {
        printf("How many of Columns :\n");
        scanf("%d",&buffer);
        // cin>>buffer;
        if(buffer<=0){
            printf("Invalid Number\n");
            continue;
        }else if(buffer>UINT8_MAX){
            printf("Number too Large\n");
            continue;
        }else{
            count=buffer;
            break;
        }
    }
    
    for(int i=0;i<count;i++)
    {
        printf("Please Input Column No.%d Name:\n",i);
        scanf("%s",col_n[i]);
        printf("Please Input Column No.%d Type('i' for int32_t, 'f' for float, 'c' fot string):\n",i);

        while(1)
        {
            scanf("\n%c",&col_t[i]);  
            if(col_t[i]!='i'&&col_t[i]!='f'&&col_t[i]!='c'){
                printf("Invalid Type. Please Retry: \n");
                continue;
            }else
                break;
        }
    }
    return count;
}


void Sync_Col_To_Mem(data_field* myMem)
{
    memcpy(myMem->raw_mem,myMem->column,myMem->column_size);
}


void pError()
{
    printf("Error: %s\n",strerror(errno));
}

void init_myMem(data_field* myMem)
{
    myMem->column_num=0;
    myMem->column_size=0;
    myMem->row_size=0;
    myMem->raw_top=0;
    // myMem->raw_mem=NULL;
    myMem->raw_mem_size=0;

    if(!(myMem->raw_mem=malloc(MAX_DATA_STORAGE_SIZE)))
        exit(-1);
    else{
        myMem->raw_mem_size=MAX_DATA_STORAGE_SIZE;
    }
}


int main()
{

    // init myMem
    data_field myMem;
    init_myMem(&myMem);
    
    // Get col_n col_t col_c
    char col_n[MAX_COLUMN][COLUMN_NAME_SIZE];
    char col_t[MAX_COLUMN];
    uint8_t col_c=getCol_info(col_n,col_t);
    
    // print col_n col_t col_c
    if(col_c==0){
        pError();
        exit(-1);
    }else{
        for(int i=0;i<col_c;i++){
            printf("No.%d Name: %s, Type: %c\n",i,col_n[i],col_t[i]);
        }
    }

    // init Table Columns
    initTable(&myMem,col_c,col_n,col_t);

    // sync myMem.column[] to myMem.raw_mem
    Sync_Col_To_Mem(&myMem);
    


    int fd;
    off_t offset;
    
    fd=open("./tmpDB",O_RDWR|O_CREAT,0644);
    if(fd==-1)
    {
        pError();
        exit(-1);
    }

    offset=lseek(fd,0,SEEK_CUR);

    printf("%d\n",offset);

    printf("raw mem size %d\n",myMem.raw_mem_size);
    printf("raw mem top %d\n",myMem.raw_top);

    write(fd,myMem.raw_mem,myMem.raw_top);

    offset=lseek(fd,0,SEEK_CUR);

    printf("%d\n",offset);


    exit(0);
}