#include"./header/rt_class.h"
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
#include"./header/ERROR.h"
#include"./header/utils.h"


#define DB_PATH "./DBFILE"

// 构造函数
DBMGR::DBMGR(){
    errno=0;
    int fd;
    if((fd=open(DB_PATH,O_RDWR|O_CREAT,0644))==-1)
        pError();
    
    this->fd=fd;
    //获取当前偏移量
    printf("now offset: %d\n",lseek(this->fd,0,SEEK_CUR));
    off_t cur_seek=lseek(this->fd,0,SEEK_END);
    printf("now offset: %d\n",lseek(this->fd,0,SEEK_CUR));
    if(cur_seek==-1)
        pError();
    // cur_seek说明是刚刚建立的新文件
    else if(cur_seek==0)
        //DBFile 未初始化
        this->InitDBFile();
    else
    {
        //非空文件
        if(lseek(this->fd,0,SEEK_SET)==-1)
            pError();

        if(read(this->fd,&this->db_header,sizeof(this->db_header))==-1)
            pError();
    }
    //重置偏移量到0
    
    if(lseek(this->fd,0,SEEK_SET)==-1)
        pError();
    if(write(this->fd,&this->db_header,sizeof(DB_Header))!=sizeof(DB_Header))
        pError("Init this.db_header ERROR");

    printf("sizeof DB_header : %d\n",sizeof(this->db_header));
    printf("sizeof tb_meta : %d\n",sizeof(this->tb_meta));
    printf("sizeof index_tb_info : %d\n",sizeof(this->idx_tb_info));
    printf("sizeof data_tb_info : %d\n",sizeof(this->data_tb_info));
    printf("sizeof index_tb : %d\n",sizeof(this->idx_tb));
    printf("sizeof data_tb : %d\n",sizeof(this->data_tb));



    // for debug
    // char tbname[]="shabi";
    // this->LoadInfo(tbname);
    // row_item row_itm[this->data_tb.col_num];

    // lseek(this->fd,this->data_tb.storage_table[0].offset,SEEK_SET);
    // read(this->fd,row_itm,sizeof(row_itm));
    // printf("%d\n",sizeof(row_itm));

}

//不允许建立空表，每个列必须指定其类型，不可为空
int DBMGR::CreateTable(char * tbname,CreateAttr *attr){
    // 先检查是否有同名的表被创建
    for(int i=0;i<this->db_header.table_num;i++)
    {
        if(!strcmp(this->db_header.clause_array[i].name,tbname))
        {
            char msg[BUF_SZ];
            sprintf(msg,"TABLE '%s' Has Been Created",tbname);
            pError(msg);
        }
    }




    this->db_header.clause_array[this->db_header.table_num].offset=lseek(this->fd,0,SEEK_END);
    strcpy(this->db_header.clause_array[this->db_header.table_num].name,tbname);

    
    //初始化 table_meta
    this->tb_meta.id=time(NULL);

    this->db_header.clause_array[this->db_header.table_num].id=this->tb_meta.id;

    strcpy(this->tb_meta.name,tbname);
    this->tb_meta.index_info_offset=lseek(this->fd,0,SEEK_END)+sizeof(this->tb_meta);
    this->tb_meta.data_info_offset=this->tb_meta.index_info_offset+sizeof(this->idx_tb_info);
    //初始化 index_table_info
    this->idx_tb_info.clause_num=0;
    this->idx_tb_info.index_offset=this->tb_meta.data_info_offset+sizeof(this->data_tb_info);
    
    // this->idx_tb_info.index_size=sizeof(this->idx_tb);
    // this->idx_tb_info.index_size=0;

    //初始化 data_table_info
    this->data_tb_info.clause_num=0;
    this->data_tb_info.data_offset=this->idx_tb_info.index_offset+sizeof(this->idx_tb);
    // this->data_tb_info.data_size=sizeof(this->data_tb);
    // this->data_tb_info.data_size=0;
    
    //索引表与数据表的 storage space 交叉存储在 data_table 之后
    this->storage_begin=this->data_tb_info.data_offset+sizeof(this->data_tb);


    // 初始化 index_table
    this->idx_tb.storage_num=0;
    this->idx_tb.storage_table[this->idx_tb.storage_num].index=this->idx_tb.storage_num;
    this->idx_tb.storage_table[this->idx_tb.storage_num].offset
        =this->storage_begin;
    this->idx_tb.storage_table[this->idx_tb.storage_num].cur_offset
        =this->idx_tb.storage_table[this->idx_tb.storage_num].offset;
    storage_index fst_index_storage;
    this->idx_tb.storage_num++;



    this->db_header.clause_array[this->db_header.table_num].index_num++;
    this->idx_tb.item_count=0;


    //初始化 data_table
    this->data_tb.storage_num=0;
    this->data_tb.storage_table[this->data_tb.storage_num].index=this->data_tb.storage_num;
    this->data_tb.storage_table[this->data_tb.storage_num].offset
        =this->storage_begin+sizeof(fst_index_storage);
    this->data_tb.storage_table[this->data_tb.storage_num].cur_offset
        =this->data_tb.storage_table[this->data_tb.storage_num].offset;
    storage_data fst_data_storage;
    this->data_tb.storage_num++;

    this->db_header.clause_array[this->db_header.table_num].data_num++;

    this->data_tb.col_num=0;


    while(true)
    {
        strcpy(this->data_tb.col_itm[this->data_tb.col_num].col_name,attr->name);
        this->data_tb.col_itm[this->data_tb.col_num].flags=attr->flags;
        this->data_tb.col_num++;

        
        if(attr->next==NULL)
            break;
        else
            attr=attr->next;
    }


    int col_primary=-1;
    for(int i=0;i<this->data_tb.col_num;i++)
    {
        //zero for false, non-zero for true
        if((this->data_tb.col_itm[i].flags&ATTR::PRIMARY))
            if(col_primary!=-1)
                pError("Too Many Primary Key");
            else
                col_primary=i;
    }
    
    if(col_primary!=-1)
    {
        index_item idx_itm;
        InitIndex(col_primary,&idx_itm);
        memcpy(&fst_index_storage,&idx_itm,sizeof(idx_itm));
        strcpy(this->idx_tb.item_info[this->idx_tb.item_count].column_name,
            this->data_tb.col_itm[col_primary].col_name);
        this->idx_tb.item_info[this->idx_tb.item_count].offset
            =this->idx_tb.storage_table[this->idx_tb.storage_num-1].cur_offset;
        this->idx_tb.storage_table[this->idx_tb.storage_num-1].cur_offset
            +=sizeof(idx_itm);
        
        //建立了索引表
        this->idx_tb_info.clause_num++;
        this->idx_tb.item_count++;
    }

    
    this->db_header.table_num++;

    if(lseek(this->fd,0,SEEK_SET)==-1)
        pWarn();
    write(this->fd,&this->db_header,sizeof(this->db_header));


    this->cur_offset_end=lseek(this->fd,0,SEEK_END);
    write(this->fd,&this->tb_meta,sizeof(this->tb_meta));
    this->cur_offset_end=lseek(this->fd,0,SEEK_CUR);
    write(this->fd,&this->idx_tb_info,sizeof(this->idx_tb_info));
    this->cur_offset_end=lseek(this->fd,0,SEEK_CUR);
    write(this->fd,&this->data_tb_info,sizeof(this->data_tb_info));
    this->cur_offset_end=lseek(this->fd,0,SEEK_CUR);
    write(this->fd,&this->idx_tb,sizeof(this->idx_tb));
    this->cur_offset_end=lseek(this->fd,0,SEEK_CUR);
    write(this->fd,&this->data_tb,sizeof(this->data_tb));
    this->cur_offset_end=lseek(this->fd,0,SEEK_CUR);
    write(this->fd,&fst_index_storage,sizeof(fst_index_storage));
    this->cur_offset_end=lseek(this->fd,0,SEEK_CUR);
    write(this->fd,&fst_data_storage,sizeof(fst_data_storage));
    this->cur_offset_end=lseek(this->fd,0,SEEK_CUR);
    // this->cur_offset_end=lseek(this->fd,0,SEEK_END);
    return 0;
}


// col_val 在声明的时候需要用 memset 清零，然后再赋值，最后传到这里
int DBMGR::Insert(char * tbname,InsertColVal *col_val){

    // 加载数据
    this->LoadInfo(tbname);
    
    // 建立 row_item 数组，暂存数据
    row_item row_itm[this->data_tb.col_num];
    val_union *primary=NULL;


    // 遍历 data_tb.col_itm[]
    for(int i=0;i<this->data_tb.col_num;i++)
    {

        row_itm[i].flags=this->data_tb.col_itm[i].flags;
        // 遍历 col_val，传过来的 值 链表
        while(true)
        {
            bool is_filled=false;
            if(!strcmp(col_val->name,this->data_tb.col_itm[i].col_name)
            &&col_val->flags==this->data_tb.col_itm[i].flags)
            {
                // row_itm[i].flags=col_val->flags;
                // memcpy()
                //直接将 value union 整个区域的值复制进来
                memcpy(&row_itm[i].value,&col_val->value,sizeof(row_itm[i].value));
                // if((row_itm[i].flags&ATTR::INT)==ATTR::INT)
                // {
                //     memcpy(&row_itm[i].value,&col_val->value,sizeof(row_itm[i].value));
                //     row_itm[i].value.i_val=col_val->value.i_val;
                // }
                // else if((row_itm[i].flags&ATTR::DOUBLE)==ATTR::DOUBLE){
                //     row_itm[i].value.d_val=col_val->value.d_val;
                // }
                // else if((row_itm[i].flags&ATTR::TIME)==ATTR::TIME){
                //     row_itm[i].value.t_val=col_val->value.t_val;
                // }
                // else if((row_itm[i].flags&ATTR::STRING)==ATTR::STRING){
                //     strcpy(row_itm[i].value.c_val,col_val->value.c_val);
                // }
                if((row_itm[i].flags&ATTR::PRIMARY)==ATTR::PRIMARY)
                {
                    primary=&row_itm[i].value;
                }
                is_filled=true;
                break;
            }
            if(!is_filled)
            {
                if((row_itm[i].flags&ATTR::INT)==ATTR::INT)
                {
                    row_itm[i].value.i_val=0;
                }
                else if((row_itm[i].flags&ATTR::DOUBLE)==ATTR::DOUBLE){
                    row_itm[i].value.d_val=0;
                }
                else if((row_itm[i].flags&ATTR::TIME)==ATTR::TIME){
                    row_itm[i].value.t_val=0;
                }
                else if((row_itm[i].flags&ATTR::STRING)==ATTR::STRING){
                    strcpy(row_itm[i].value.c_val,"");
                }
            }
            // is_filled=false;

            if(col_val->next==NULL)
                break;
            else
                col_val=col_val->next;
        }
    }
    if(primary==NULL)
        pError("Lost Primary Key");
    
    // 将 row_item 数组中的数据写入文件
    // off_t storage_offset=this->data_tb.storage_table[this->data_tb.storage_num-1].offset;
    if(lseek(this->fd,this->data_tb.storage_table[this->data_tb.storage_num-1].cur_offset,SEEK_SET)==-1)
        pError();
    if(write(this->fd,row_itm,sizeof(row_itm))==-1)
        pError();
    
    // 更新data_tb数值
    this->data_tb.storage_table[this->data_tb.storage_num-1].cur_offset
        +=sizeof(row_itm);
    
    this->data_tb_info.clause_num++;



    // 添加到索引
    index_item idx_item;
    for(int i=0;i<this->data_tb.col_num;i++)
    {
        // 找到该表的主键
        if((this->data_tb.col_itm[i].flags&ATTR::PRIMARY)==ATTR::PRIMARY)
        {
            for(int j=0;j<this->idx_tb.item_count;j++)
            {
                // 找到主键对应的索引
                if(!strcmp(this->idx_tb.item_info[j].column_name,this->data_tb.col_itm[i].col_name))
                {
                    // 读取索引 idx_item
                    if(lseek(this->fd,this->idx_tb.item_info[j].offset,SEEK_SET)==-1)
                        pError();
                    if(read(this->fd,&idx_item,sizeof(idx_item))==-1)
                        pError();

                    // 获取主键 value 的 hashed 值 0-26
                    uint hashed=Hash::Murmur(primary,sizeof(val_union))%27;

                    // 将该行加入索引树
                    // this->Distribute(hashed,idx_item.tree,
                    //     this->data_tb.storage_table[this->data_tb.storage_num-1]
                    //         .cur_offset-sizeof(row_item));
                    this->Distribute(primary,hashed,&idx_item,
                        this->data_tb.storage_table[this->data_tb.storage_num-1]
                            .cur_offset-sizeof(row_item));


                    // 将 idx_item 写入dbFile
                    if(lseek(this->fd,this->idx_tb.item_info[j].offset,SEEK_SET)==-1)
                        pError();
                    if(write(this->fd,&idx_item,sizeof(idx_item))==-1)
                        pError();

                    
                    break;
                }
            }
            break;
        }
    }
    
    // idx_tb, idx_tb_info 在此函数并并为修改，不用写回
    // this->idx_tb;

    // 写回 data_tb
    if(lseek(this->fd,this->data_tb_info.data_offset,SEEK_SET)==-1)
        pError();
    if(write(this->fd,&this->data_tb,sizeof(this->data_tb))==-1)
        pError();
    
    // 写回 data_tb_info
    if(lseek(this->fd,this->tb_meta.data_info_offset,SEEK_SET)==-1)
        pError();
    if(write(this->fd,&this->data_tb_info,sizeof(this->data_tb_info))==-1)
        pError();
    
    // table_meta, DB_Header 都没有修改，不写回

    return 0;
}


// condi 可以为NULL
char * DBMGR::Select(char * tbname,SelectCol col,SelectCondi *condi){
    
    // 最终完全匹配的偏移量
    std::stack<off_t> offset_final;
    // 加载数据
    this->LoadInfo(tbname);


    if(condi!=NULL)
    {


        // 遍历condi，尝试获取 idx_item 注意：本索引由于是根据 value 的 hash 值 建立的
        // 所以仅针对 NEQUAL EQUAL 有效
        UpdataCondi *workptr=condi;
        bool found_value2data_table=false;
        index_item idx_itm;
        //用于更新索引后写回
        off_t idx_itm_offset;
        while(true)
        {

            if(workptr->opt==NEQUAL||workptr->opt==EQUAL)
            {
                // 遍历 idx_tb.item_info
                for(int i=0;i<this->idx_tb.item_count;i++)
                {
                    // 找到第一个索引列
                    if(!strcmp(this->idx_tb.item_info[i].column_name,workptr->first))
                    {
                        found_value2data_table=true;
                        if(lseek(this->fd,this->idx_tb.item_info[i].offset,SEEK_SET)==-1)
                            pError();
                        if(read(this->fd,&idx_itm,sizeof(idx_itm))==-1)
                            pError();
                        break;
                    }
                }
            }
            
            if(found_value2data_table)
                break;
            else if(workptr->next==NULL)
                break;
            else
                workptr=workptr->next;
        }
        






        // row_itm[] 用于保存某一行的数据
        row_item row_itm[this->data_tb.col_num];

        // 获取索引成功 当前索引树保存在 idx_itm ， 索引 value 在 workptr->second
        if(found_value2data_table)
        {
            // 将 value2data_table 中匹配的偏移量保存到 offset_after_index 中
            std::stack<off_t> offset_after_index;
            
            value2data_table matched_table=this->LocateWithIndex(idx_itm,workptr->second);
            for(int i=0;i<matched_table.count;i++)
            {
                if(!memcmp(&matched_table.value2data[i].value,&workptr->second,sizeof(val_union)))
                {
                    offset_after_index.push(matched_table.value2data[i].offset);
                }
            }
            // 未找到匹配项
            if(offset_after_index.size()!=0)
            {
            // 找到若干匹配项，其偏移量保存在 offset_after_index 中
                while(!offset_after_index.empty())
                {
                    int condi_depth_cnt=0;
                    int condi_match_cnt=0;
                    //将其读入row_itm
                    if(lseek(this->fd,offset_after_index.top(),SEEK_SET)==-1)
                        pError();
                    if(read(this->fd,row_itm,sizeof(row_itm))==-1)
                        pError();
                    

                    // 遍历 col_item[]
                    for(int i=0;i<this->data_tb.col_num;i++)
                    {
                        workptr=condi;
                        // 遍历 condi 链表
                        while(true)
                        {
                            condi_depth_cnt++;
                            // 如果 当前 workptr 指向的列数据 与 col_item[i] 匹配
                            if(!strcmp(this->data_tb.col_itm[i].col_name,workptr->first))
                            {
                                if(this->data_tb.col_itm[i].flags&&ATTR::INT==ATTR::INT)
                                {
                                    switch (workptr->opt){
                                        case NEQUAL:{
                                            if(workptr->second.i_val!=row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case LESS:{
                                            if(workptr->second.i_val<row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case LEQUAL:{
                                            if(workptr->second.i_val<=row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case EQUAL:{
                                            if(workptr->second.i_val==row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case GEUQAL:{
                                            if(workptr->second.i_val>=row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case GREATER:{
                                            if(workptr->second.i_val>row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        default:{
                                            pError("Invalid Operator");
                                        }
                                    }
                                }
                                else if(this->data_tb.col_itm[i].flags&&ATTR::DOUBLE==ATTR::DOUBLE){
                                    switch (workptr->opt){
                                        case NEQUAL:{
                                            if(fabs(workptr->second.d_val-row_itm[i].value.d_val)>EPS)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case LESS:{
                                            if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                            row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case LEQUAL:{
                                            if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                            row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case EQUAL:{
                                            if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case GEUQAL:{
                                            if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                            workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case GREATER:{
                                            if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                            workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        default:{
                                            pError("Invalid Operator");
                                        }
                                    }
                                }
                                else if(this->data_tb.col_itm[i].flags&&ATTR::STRING==ATTR::STRING){
                                    switch (workptr->opt){
                                        case NEQUAL:{
                                            if(strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case EQUAL:{
                                            if(!strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        default:{
                                            pError("Invalid Operator");
                                        }
                                    }
                                }
                                else if(this->data_tb.col_itm[i].flags&&ATTR::TIME==ATTR::TIME){
                                    switch (workptr->opt){
                                        case NEQUAL:{
                                            if(workptr->second.i_val!=row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case LESS:{
                                            if(workptr->second.i_val<row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case LEQUAL:{
                                            if(workptr->second.i_val<=row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case EQUAL:{
                                            if(workptr->second.i_val==row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case GEUQAL:{
                                            if(workptr->second.i_val>=row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        case GREATER:{
                                            if(workptr->second.i_val>row_itm[i].value.i_val)
                                                condi_match_cnt++;
                                            else
                                                break;
                                        }
                                        default:{
                                            pError("Invalid Operator");
                                        }
                                    }
                                }
                            }
                            if(workptr==NULL)
                                break;
                        }
                        if(condi_depth_cnt!=condi_match_cnt)
                            break;
                    }
                    // 如果condi depth cnt 为 0， 则说明DBFile出错
                    if(condi_depth_cnt==0)
                        pError("Table Broken");
                    else if(condi_depth_cnt==condi_match_cnt)
                        offset_final.push(offset_after_index.top());

                    offset_after_index.pop();
                }
            }

        }
        else{
        // 获取索引失败，则逐行扫描
            off_t cur_offset=this->data_tb.storage_table[0].offset;
            for(int i=0;i<this->data_tb.col_num;i++)
            {
                int condi_depth_cnt=0;
                int condi_match_cnt=0;
                
                
                // 读取一行数据到row_itm
                if(lseek(this->fd,cur_offset,SEEK_SET)==-1)
                    pError();
                if(read(this->fd,row_itm,sizeof(row_itm))==-1)
                    pError();
                
                // 遍历 col_item[]
                for(int i=0;i<this->data_tb.col_num;i++)
                {
                    workptr=condi;
                    // 遍历 condi 链表
                    while(true)
                    {
                        condi_depth_cnt++;
                        // 如果 当前 workptr 指向的列数据 与 col_item[i] 匹配
                        if(!strcmp(this->data_tb.col_itm[i].col_name,workptr->first))
                        {
                            if(this->data_tb.col_itm[i].flags&&ATTR::INT==ATTR::INT)
                            {
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(workptr->second.i_val!=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(workptr->second.i_val<row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(workptr->second.i_val<=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(workptr->second.i_val==row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(workptr->second.i_val>=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(workptr->second.i_val>row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::DOUBLE==ATTR::DOUBLE){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::STRING==ATTR::STRING){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(!strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::TIME==ATTR::TIME){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(workptr->second.i_val!=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(workptr->second.i_val<row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(workptr->second.i_val<=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(workptr->second.i_val==row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(workptr->second.i_val>=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(workptr->second.i_val>row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                        }
                        if(workptr==NULL)
                            break;
                    }
                    if(condi_depth_cnt!=condi_match_cnt)
                        break;
                }
                // 如果condi depth cnt 为 0， 则说明DBFile出错
                if(condi_depth_cnt==0)
                    pError("Table Broken");
                else if(condi_depth_cnt==condi_match_cnt)
                    offset_final.push(cur_offset);

                // 更新 cur_offset 值
                cur_offset+=sizeof(row_itm);
                
            }
        }


        
    }
    else{
        // condi 为空
        int row_cnt=(this->data_tb.storage_table[0].cur_offset-this->data_tb.storage_table[0].offset)/sizeof(val_union);
        off_t offset=this->data_tb.storage_table[0].offset;
        for(int i=0;i<row_cnt;i++)
        {
            offset_final.push(offset);
            offset+=sizeof(val_union);
        }
    }




    //查找结束，选择

    char *str_swapper=(char *)malloc(BUF_SZ*sizeof(char));
    std::string string_swapper;

    SelectCol *workcolptr=&col;

    std::queue<int> col_idx;

    while(true)
    {
        sprintf(str_swapper,"%s    ",workcolptr->name);
        string_swapper.append(string_swapper);
        
        for(int i=0;i<this->data_tb.col_num;i++)
        {
            if(!strcmp(this->data_tb.col_itm[i].col_name,workcolptr->name))
            {
                col_idx.push(i);
                break;
            }
        }

        if(col.next==NULL)
        {
            string_swapper.append("\n");
            break;
        }
    }

    while(!offset_final.empty())
    {
        std::queue<int> col_idx_copy=col_idx;

        if(lseek(this->fd,offset_final.top(),SEEK_SET)==-1)
                pError();
        if(read(this->fd,row_itm,sizeof(row_itm))==-1)
            pError();

        while(!col_idx_copy.empty())
        {
            sprintf(str_swapper,"%s    ",col_idx_copy.front());
            col_idx_copy.pop();
        }
        string_swapper.append("\n");
    }

    strcpy(str_swapper,string_swapper.c_str());

    return str_swapper;

    // 遍历 select_condition，查找是否存在索引
    // 若条件中存在多个索引，则只有第一个索引生效
    // 仅通过索引可能产生多个匹配项的偏移量，遍历这些较少的匹配项来检查是否符合其他列的条件
    // 能通过索引表检查的运算规则只有等于

}

char * DBMGR::ShowTables(){
    // 遍历 DB_Header 的 clause_arrary 即可获得所有的表名
    std::string table_names;

    for(int i=0;i<this->db_header.table_num;i++)
    {
        table_names.append(this->db_header.clause_array[i].name);
        table_names.append(" ");
    }

    char *res=(char *)malloc(sizeof(char)*BUF_SZ);
    strcpy(res,table_names.c_str());
    printf("%s\n",res);
    return res;
}

char * DBMGR::ShowColumns(char * tbname){
    // DB_Header ==> table_meta ==> data_table_info ==> data_table ==> |列数量，列名字，列属性|
    
    // 加载数据
    this->LoadInfo(tbname);

    char *str_swaper=(char *)malloc(BUF_SZ*sizeof(char));
    std::string col_info;
    sprintf(str_swaper,"COLUMN NUM : %d\n",this->data_tb.col_num);
    col_info.append(str_swaper);

    for(int i=0;i<this->data_tb.col_num;i++)
    {
        strcpy(str_swaper,this->data_tb.col_itm[i].col_name);
        col_info.append(str_swaper);
        if((this->data_tb.col_itm[i].flags&ATTR::INT)==ATTR::INT)
            col_info.append(" INT");
        if((this->data_tb.col_itm[i].flags&ATTR::DOUBLE)==ATTR::DOUBLE)
            col_info.append(" DOUBLE");
        if((this->data_tb.col_itm[i].flags&ATTR::STRING)==ATTR::STRING)
            col_info.append(" STRING");
        if((this->data_tb.col_itm[i].flags&ATTR::TIME)==ATTR::TIME)
            col_info.append(" TIME");
        if((this->data_tb.col_itm[i].flags&ATTR::AUTOINC)==ATTR::AUTOINC)
            col_info.append(" AUTOINC");
        if((this->data_tb.col_itm[i].flags&ATTR::PRIMARY)==ATTR::PRIMARY)
            col_info.append(" PRIMARY");
        if((this->data_tb.col_itm[i].flags&ATTR::INDEX)==ATTR::INDEX)
            col_info.append(" INDEX");
        

        // 返回的字符串最后不要换行符，调用此函数动态添加
        // col_info.append("\n");
    }

    strcpy(str_swaper,col_info.c_str());
    printf("%s\n",str_swaper);
    return str_swaper;
}

char * DBMGR::ShowIndex(char * tbname){
    // DB_Header ==> table_meta|表名，唯一识别id|  ==> index_table_info ==> index_table |索引数量，索引列名|
    //                          ==> data_table_info ==> data_table |列数量，列名字，列属性|

    //加载数据
    this->LoadInfo(tbname);

    char *str_swaper=(char *)malloc(BUF_SZ*sizeof(char));
    std::string string_info;

    sprintf(str_swaper,"TABLE NAME : %s\nID : %d\n",tbname,this->tb_meta.id);
    string_info.append(str_swaper);

    sprintf(str_swaper,"INDEX TABLE NUM : %d\n",this->idx_tb.item_count);
    string_info.append(str_swaper);


    string_info.append("INDEXED COLUMN : ");
    for(int i=0;i<this->idx_tb.item_count;i++)
    {
        sprintf(str_swaper," %s",this->idx_tb.item_info[i].column_name);
        string_info.append(str_swaper);
    }
    string_info.append("\n");
    

    sprintf(str_swaper,"COLUMN NUM : %d\n",this->data_tb.col_num);
    string_info.append(str_swaper);

    for(int i=0;i<this->data_tb.col_num;i++)
    {
        strcpy(str_swaper,this->data_tb.col_itm[i].col_name);
        string_info.append(str_swaper);
        if((this->data_tb.col_itm[i].flags&ATTR::INT)==ATTR::INT)
            string_info.append(" INT");
        if((this->data_tb.col_itm[i].flags&ATTR::DOUBLE)==ATTR::DOUBLE)
            string_info.append(" DOUBLE");
        if((this->data_tb.col_itm[i].flags&ATTR::STRING)==ATTR::STRING)
            string_info.append(" STRING");
        if((this->data_tb.col_itm[i].flags&ATTR::TIME)==ATTR::TIME)
            string_info.append(" TIME");
        if((this->data_tb.col_itm[i].flags&ATTR::AUTOINC)==ATTR::AUTOINC)
            string_info.append(" AUTOINC");
        if((this->data_tb.col_itm[i].flags&ATTR::PRIMARY)==ATTR::PRIMARY)
            string_info.append(" PRIMARY");
        if((this->data_tb.col_itm[i].flags&ATTR::INDEX)==ATTR::INDEX)
            string_info.append(" INDEX");
        
        string_info.append("\n");
    }


    strcpy(str_swaper,string_info.c_str());

    printf("%s\n",str_swaper);

    return str_swaper;

}

// condi 可以为NULL
int DBMGR::Update(char * tbname,char *colname,val_union colvalue,UpdataCondi *condi){
    // 修改成功的个数
    uint8_t changed_count=0;

    // 加载数据
    this->LoadInfo(tbname);

    // 遍历condi，尝试获取 idx_item 注意：本索引由于是根据 value 的 hash 值 建立的
    // 所以仅针对 NEQUAL EQUAL 有效
    UpdataCondi *workptr=condi;
    bool found_value2data_table=false;
    index_item idx_itm;
    //用于更新索引后写回
    off_t idx_itm_offset;
    while(true)
    {
        if(workptr->opt==NEQUAL||workptr->opt==EQUAL)
        {
            // 遍历 idx_tb.item_info
            for(int i=0;i<this->idx_tb.item_count;i++)
            {
                // 找到第一个索引列
                if(!strcmp(this->idx_tb.item_info[i].column_name,workptr->first))
                {
                    found_value2data_table=true;
                    if(lseek(this->fd,this->idx_tb.item_info[i].offset,SEEK_SET)==-1)
                        pError();
                    if(read(this->fd,&idx_itm,sizeof(idx_itm))==-1)
                        pError();
                    break;
                }
            }
        }
        
        if(found_value2data_table)
            break;
        else if(workptr->next==NULL)
            break;
        else
            workptr=workptr->next;
    }
    

    // 最终完全匹配的偏移量
        std::stack<off_t> offset_final;
    // row_itm[] 用于保存某一行的数据
    row_item row_itm[this->data_tb.col_num];

    // 获取索引成功 当前索引树保存在 idx_itm ， 索引 value 在 workptr->second
    if(found_value2data_table)
    {
        // 将 value2data_table 中匹配的偏移量保存到 offset_after_index 中
        std::stack<off_t> offset_after_index;
        
        value2data_table matched_table=this->LocateWithIndex(idx_itm,workptr->second);
        for(int i=0;i<matched_table.count;i++)
        {
            if(!memcmp(&matched_table.value2data[i].value,&workptr->second,sizeof(val_union)))
            {
                offset_after_index.push(matched_table.value2data[i].offset);
            }
        }
        // 未找到匹配项
        if(offset_after_index.size()==0)
            changed_count=0;
        else{
        // 找到若干匹配项，其偏移量保存在 offset_after_index 中
            while(!offset_after_index.empty())
            {
                int condi_depth_cnt=0;
                int condi_match_cnt=0;
                //将其读入row_itm
                if(lseek(this->fd,offset_after_index.top(),SEEK_SET)==-1)
                    pError();
                if(read(this->fd,row_itm,sizeof(row_itm))==-1)
                    pError();
                

                // 遍历 col_item[]
                for(int i=0;i<this->data_tb.col_num;i++)
                {
                    workptr=condi;
                    // 遍历 condi 链表
                    while(true)
                    {
                        condi_depth_cnt++;
                        // 如果 当前 workptr 指向的列数据 与 col_item[i] 匹配
                        if(!strcmp(this->data_tb.col_itm[i].col_name,workptr->first))
                        {
                            if(this->data_tb.col_itm[i].flags&&ATTR::INT==ATTR::INT)
                            {
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(workptr->second.i_val!=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(workptr->second.i_val<row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(workptr->second.i_val<=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(workptr->second.i_val==row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(workptr->second.i_val>=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(workptr->second.i_val>row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::DOUBLE==ATTR::DOUBLE){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::STRING==ATTR::STRING){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(!strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::TIME==ATTR::TIME){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(workptr->second.i_val!=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(workptr->second.i_val<row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(workptr->second.i_val<=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(workptr->second.i_val==row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(workptr->second.i_val>=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(workptr->second.i_val>row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                        }
                        if(workptr==NULL)
                            break;
                    }
                    if(condi_depth_cnt!=condi_match_cnt)
                        break;
                }
                // 如果condi depth cnt 为 0， 则说明DBFile出错
                if(condi_depth_cnt==0)
                    pError("Table Broken");
                else if(condi_depth_cnt==condi_match_cnt)
                    offset_final.push(offset_after_index.top());

                offset_after_index.pop();
            }
        }

    }
    else{
    // 获取索引失败，则逐行扫描
        off_t cur_offset=this->data_tb.storage_table[0].offset;
        for(int i=0;i<this->data_tb.col_num;i++)
        {
            int condi_depth_cnt=0;
            int condi_match_cnt=0;
            
            
            // 读取一行数据到row_itm
            if(lseek(this->fd,cur_offset,SEEK_SET)==-1)
                pError();
            if(read(this->fd,row_itm,sizeof(row_itm))==-1)
                pError();
            
            // 遍历 col_item[]
            for(int i=0;i<this->data_tb.col_num;i++)
            {
                workptr=condi;
                // 遍历 condi 链表
                while(true)
                {
                    condi_depth_cnt++;
                    // 如果 当前 workptr 指向的列数据 与 col_item[i] 匹配
                    if(!strcmp(this->data_tb.col_itm[i].col_name,workptr->first))
                    {
                        if(this->data_tb.col_itm[i].flags&&ATTR::INT==ATTR::INT)
                        {
                            switch (workptr->opt){
                                case NEQUAL:{
                                    if(workptr->second.i_val!=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LESS:{
                                    if(workptr->second.i_val<row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LEQUAL:{
                                    if(workptr->second.i_val<=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case EQUAL:{
                                    if(workptr->second.i_val==row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GEUQAL:{
                                    if(workptr->second.i_val>=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GREATER:{
                                    if(workptr->second.i_val>row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                default:{
                                    pError("Invalid Operator");
                                }
                            }
                        }
                        else if(this->data_tb.col_itm[i].flags&&ATTR::DOUBLE==ATTR::DOUBLE){
                            switch (workptr->opt){
                                case NEQUAL:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LESS:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                    row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LEQUAL:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                    row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case EQUAL:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GEUQAL:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                    workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GREATER:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                    workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                default:{
                                    pError("Invalid Operator");
                                }
                            }
                        }
                        else if(this->data_tb.col_itm[i].flags&&ATTR::STRING==ATTR::STRING){
                            switch (workptr->opt){
                                case NEQUAL:{
                                    if(strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case EQUAL:{
                                    if(!strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                default:{
                                    pError("Invalid Operator");
                                }
                            }
                        }
                        else if(this->data_tb.col_itm[i].flags&&ATTR::TIME==ATTR::TIME){
                            switch (workptr->opt){
                                case NEQUAL:{
                                    if(workptr->second.i_val!=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LESS:{
                                    if(workptr->second.i_val<row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LEQUAL:{
                                    if(workptr->second.i_val<=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case EQUAL:{
                                    if(workptr->second.i_val==row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GEUQAL:{
                                    if(workptr->second.i_val>=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GREATER:{
                                    if(workptr->second.i_val>row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                default:{
                                    pError("Invalid Operator");
                                }
                            }
                        }
                    }
                    if(workptr==NULL)
                        break;
                }
                if(condi_depth_cnt!=condi_match_cnt)
                    break;
            }
            // 如果condi depth cnt 为 0， 则说明DBFile出错
            if(condi_depth_cnt==0)
                pError("Table Broken");
            else if(condi_depth_cnt==condi_match_cnt)
                offset_final.push(cur_offset);

            // 更新 cur_offset 值
            cur_offset+=sizeof(row_itm);
            
        }
    }

    //查找结束，开始更新值

    for(int i=0;i<this->data_tb.col_num;i++)
    {
        if(!strcmp(this->data_tb.col_itm[i].col_name,colname))
        {
            while(!offset_final.empty())
            {
                //将其读入row_itm
                if(lseek(this->fd,offset_final.top(),SEEK_SET)==-1)
                    pError();
                if(read(this->fd,row_itm,sizeof(row_itm))==-1)
                    pError();
                
                // 将值写入 row_itm
                memcpy(&row_itm[i].value,&colvalue,sizeof(val_union));

                // 写回 DBFile
                if(lseek(this->fd,offset_final.top(),SEEK_SET)==-1)
                    pError();
                if(write(this->fd,row_itm,sizeof(row_itm))==-1)
                    pError();
                
                // 暂时未能找到删除之前索引的办法，忽略掉，对于短时间、小数据，应该不会影响
                if(row_itm[i].flags&&ATTR::PRIMARY==ATTR::PRIMARY||
                row_itm[i].flags&&ATTR::INDEX==ATTR::INDEX)
                {
                    uint hashed=Hash::Murmur(&colvalue,sizeof(val_union))%27;
                    this->Distribute(&colvalue,hashed,&idx_itm,offset_final.top());
                }

                // 将 索引 写回 DBFile
                if(lseek(this->fd,idx_itm_offset,SEEK_SET)==-1)
                    pError();
                if(write(this->fd,&idx_itm,sizeof(idx_itm))==-1)
                    pError();


                // 修改计数
                changed_count++;
                
                offset_final.pop();
            }
        }
    }


    return changed_count;


    // 即修改数据的值
    // 检查condition中是否存在索引列
    // 若有则找到部分匹配项之后遍历，获得数据保存的偏移量，进行修改后，更新索引表 Distribute()，原来的索引项如何处理未确定
    // 若无则遍历表的所有项，进行修改后，更新索引表 Distribute()，原来的索引项如何处理未确定
}

int DBMGR::DropRow(char * tbname,DropRowCondi *condi){
    // 修改成功的个数
    uint8_t changed_count=0;

    // 加载数据
    this->LoadInfo(tbname);

    // 遍历condi，尝试获取 idx_item 注意：本索引由于是根据 value 的 hash 值 建立的
    // 所以仅针对 NEQUAL EQUAL 有效
    UpdataCondi *workptr=condi;
    bool found_value2data_table=false;
    index_item idx_itm;
    //用于更新索引后写回
    off_t idx_itm_offset;
    while(true)
    {
        if(workptr->opt==NEQUAL||workptr->opt==EQUAL)
        {
            // 遍历 idx_tb.item_info
            for(int i=0;i<this->idx_tb.item_count;i++)
            {
                // 找到第一个索引列
                if(!strcmp(this->idx_tb.item_info[i].column_name,workptr->first))
                {
                    found_value2data_table=true;
                    if(lseek(this->fd,this->idx_tb.item_info[i].offset,SEEK_SET)==-1)
                        pError();
                    if(read(this->fd,&idx_itm,sizeof(idx_itm))==-1)
                        pError();
                    break;
                }
            }
        }
        
        if(found_value2data_table)
            break;
        else if(workptr->next==NULL)
            break;
        else
            workptr=workptr->next;
    }
    

    // 最终完全匹配的偏移量
        std::stack<off_t> offset_final;
    // row_itm[] 用于保存某一行的数据
    row_item row_itm[this->data_tb.col_num];

    // 获取索引成功 当前索引树保存在 idx_itm ， 索引 value 在 workptr->second
    if(found_value2data_table)
    {
        // 将 value2data_table 中匹配的偏移量保存到 offset_after_index 中
        std::stack<off_t> offset_after_index;
        
        value2data_table matched_table=this->LocateWithIndex(idx_itm,workptr->second);
        for(int i=0;i<matched_table.count;i++)
        {
            if(!memcmp(&matched_table.value2data[i].value,&workptr->second,sizeof(val_union)))
            {
                offset_after_index.push(matched_table.value2data[i].offset);
            }
        }
        // 未找到匹配项
        if(offset_after_index.size()==0)
            changed_count=0;
        else{
        // 找到若干匹配项，其偏移量保存在 offset_after_index 中
            while(!offset_after_index.empty())
            {
                int condi_depth_cnt=0;
                int condi_match_cnt=0;
                //将其读入row_itm
                if(lseek(this->fd,offset_after_index.top(),SEEK_SET)==-1)
                    pError();
                if(read(this->fd,row_itm,sizeof(row_itm))==-1)
                    pError();
                

                // 遍历 col_item[]
                for(int i=0;i<this->data_tb.col_num;i++)
                {
                    workptr=condi;
                    // 遍历 condi 链表
                    while(true)
                    {
                        condi_depth_cnt++;
                        // 如果 当前 workptr 指向的列数据 与 col_item[i] 匹配
                        if(!strcmp(this->data_tb.col_itm[i].col_name,workptr->first))
                        {
                            if(this->data_tb.col_itm[i].flags&&ATTR::INT==ATTR::INT)
                            {
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(workptr->second.i_val!=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(workptr->second.i_val<row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(workptr->second.i_val<=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(workptr->second.i_val==row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(workptr->second.i_val>=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(workptr->second.i_val>row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::DOUBLE==ATTR::DOUBLE){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                        workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::STRING==ATTR::STRING){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(!strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                            else if(this->data_tb.col_itm[i].flags&&ATTR::TIME==ATTR::TIME){
                                switch (workptr->opt){
                                    case NEQUAL:{
                                        if(workptr->second.i_val!=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LESS:{
                                        if(workptr->second.i_val<row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case LEQUAL:{
                                        if(workptr->second.i_val<=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case EQUAL:{
                                        if(workptr->second.i_val==row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GEUQAL:{
                                        if(workptr->second.i_val>=row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    case GREATER:{
                                        if(workptr->second.i_val>row_itm[i].value.i_val)
                                            condi_match_cnt++;
                                        else
                                            break;
                                    }
                                    default:{
                                        pError("Invalid Operator");
                                    }
                                }
                            }
                        }
                        if(workptr==NULL)
                            break;
                    }
                    if(condi_depth_cnt!=condi_match_cnt)
                        break;
                }
                // 如果condi depth cnt 为 0， 则说明DBFile出错
                if(condi_depth_cnt==0)
                    pError("Table Broken");
                else if(condi_depth_cnt==condi_match_cnt)
                    offset_final.push(offset_after_index.top());

                offset_after_index.pop();
            }
        }

    }
    else{
    // 获取索引失败，则逐行扫描
        off_t cur_offset=this->data_tb.storage_table[0].offset;
        for(int i=0;i<this->data_tb.col_num;i++)
        {
            int condi_depth_cnt=0;
            int condi_match_cnt=0;
            
            
            // 读取一行数据到row_itm
            if(lseek(this->fd,cur_offset,SEEK_SET)==-1)
                pError();
            if(read(this->fd,row_itm,sizeof(row_itm))==-1)
                pError();
            
            // 遍历 col_item[]
            for(int i=0;i<this->data_tb.col_num;i++)
            {
                workptr=condi;
                // 遍历 condi 链表
                while(true)
                {
                    condi_depth_cnt++;
                    // 如果 当前 workptr 指向的列数据 与 col_item[i] 匹配
                    if(!strcmp(this->data_tb.col_itm[i].col_name,workptr->first))
                    {
                        if(this->data_tb.col_itm[i].flags&&ATTR::INT==ATTR::INT)
                        {
                            switch (workptr->opt){
                                case NEQUAL:{
                                    if(workptr->second.i_val!=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LESS:{
                                    if(workptr->second.i_val<row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LEQUAL:{
                                    if(workptr->second.i_val<=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case EQUAL:{
                                    if(workptr->second.i_val==row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GEUQAL:{
                                    if(workptr->second.i_val>=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GREATER:{
                                    if(workptr->second.i_val>row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                default:{
                                    pError("Invalid Operator");
                                }
                            }
                        }
                        else if(this->data_tb.col_itm[i].flags&&ATTR::DOUBLE==ATTR::DOUBLE){
                            switch (workptr->opt){
                                case NEQUAL:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LESS:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                    row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LEQUAL:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                    row_itm[i].value.d_val-workptr->second.d_val>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case EQUAL:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GEUQAL:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                    workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GREATER:{
                                    if(fabs(workptr->second.d_val-row_itm[i].value.d_val)<EPS||
                                    workptr->second.d_val-row_itm[i].value.d_val>EPS)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                default:{
                                    pError("Invalid Operator");
                                }
                            }
                        }
                        else if(this->data_tb.col_itm[i].flags&&ATTR::STRING==ATTR::STRING){
                            switch (workptr->opt){
                                case NEQUAL:{
                                    if(strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case EQUAL:{
                                    if(!strcmp(workptr->second.c_val,row_itm[i].value.c_val))
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                default:{
                                    pError("Invalid Operator");
                                }
                            }
                        }
                        else if(this->data_tb.col_itm[i].flags&&ATTR::TIME==ATTR::TIME){
                            switch (workptr->opt){
                                case NEQUAL:{
                                    if(workptr->second.i_val!=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LESS:{
                                    if(workptr->second.i_val<row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case LEQUAL:{
                                    if(workptr->second.i_val<=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case EQUAL:{
                                    if(workptr->second.i_val==row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GEUQAL:{
                                    if(workptr->second.i_val>=row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                case GREATER:{
                                    if(workptr->second.i_val>row_itm[i].value.i_val)
                                        condi_match_cnt++;
                                    else
                                        break;
                                }
                                default:{
                                    pError("Invalid Operator");
                                }
                            }
                        }
                    }
                    if(workptr==NULL)
                        break;
                }
                if(condi_depth_cnt!=condi_match_cnt)
                    break;
            }
            // 如果condi depth cnt 为 0， 则说明DBFile出错
            if(condi_depth_cnt==0)
                pError("Table Broken");
            else if(condi_depth_cnt==condi_match_cnt)
                offset_final.push(cur_offset);

            // 更新 cur_offset 值
            cur_offset+=sizeof(row_itm);
            
        }
    }



    //查找结束，开始删除数据行

    off_t row_offset;


    // 将 row_itm 清零，用于清零覆盖文件内容
    memset(row_itm,0,sizeof(row_itm));

    while(!offset_final.empty())
    {
        if(lseek(this->fd,offset_final.top(),SEEK_SET)==-1)
            pError();
        if(write(this->fd,row_itm,sizeof(row_itm))==-1)
            pError();

        changed_count++;
        offset_final.pop();
    }


    return changed_count;

    // 即将 data_table 中对应数据行删除掉，并修改对应的 统计数据
    // 但是删除索引的方法还未明确，所以暂时不修改 index_table 的内容

}


// int DBMGR::DropTable(char * tbname){
//     // 删除表
//     // 如果删除文件中间的内容，填0？还是想办法形成文件空洞？
// }

void DBMGR::Close()
{
    if(close(this->fd)==-1)
        pWarn("Close Fd Failed");
    else
        printf("Close Success\n");
    
}


