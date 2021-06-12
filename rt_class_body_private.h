#include"rt_class.h"



value2data_table DBMGR::LocateWithIndex(index_item idx_itm, val_union value)
{
    uint8_t hashed=Hash::Murmur(&value,sizeof(val_union))%27;
    return idx_itm.table[hashed];
}



// 加载数据
void DBMGR::LoadInfo(char * tbname)
{
    // 表是否存在
    bool is_exist=false;
    off_t table_meta_offset=0;
    for(int i=0;i<this->db_header.table_num;i++)
    {
        if(!strcmp(this->db_header.clause_array[i].name,tbname))
        {
            table_meta_offset=this->db_header.clause_array[i].offset;
            is_exist=true;
            break;
        }
    }

    if(!is_exist)
        // 表不存在
        pError("Table Not Exists");
    
    // 读取 tb_meta
    if(lseek(this->fd,table_meta_offset,SEEK_SET))
        pError();
    if(read(this->fd,&this->tb_meta,sizeof(this->tb_meta))==-1)
        pError();
    
    // 读取 data_tb_info
    if(lseek(this->fd,this->tb_meta.data_info_offset,SEEK_SET)==-1)
        pError();
    if(read(this->fd,&this->data_tb_info,sizeof(this->data_tb_info))==-1)
        pError();

    // 读取 data_tb
    if(lseek(this->fd,this->data_tb_info.data_offset,sizeof(this->data_tb_info))==-1)
        pError();
    if(read(this->fd,&this->data_tb,sizeof(this->data_tb))==-1)
        pError();

    // 读取 index_tb_info
    if(lseek(this->fd,this->tb_meta.index_info_offset,SEEK_SET)==-1)
        pError();
    if(read(this->fd,&this->idx_tb_info,sizeof(this->idx_tb_info))==-1)
        pError();

    // 读取 index_tb
    if(lseek(this->fd,this->idx_tb_info.index_offset,SEEK_SET)==-1)
        pError();
    if(read(this->fd,&this->idx_tb,sizeof(this->idx_tb))==-1)
        pError();
}






void DBMGR::InitDBFile()
{
    // header->table_num=0;
    this->db_header.table_num=0;
    
    if(write(this->fd,&this->db_header,sizeof(DB_Header))==-1)
        pError();
}





void DBMGR::InitIndex(uint8_t col_idx,index_item * idx_itm)
{
    // index_item idx_itm;
    GetTribleTree(idx_itm->tree);
    strcpy(idx_itm->column_name,this->data_tb.col_itm[col_idx].col_name);
    idx_itm->flags=this->data_tb.col_itm[col_idx].flags;

    for(int i=0;i<TRIBLE_TREE_LEAF_LENGTH;i++)
    {
        idx_itm->table[i].count=0;
    }

}


// 将对应的数据偏移量存放到对应的存储区中
// hashed 0-26
// 设计漏洞，没能用上树 :(
void DBMGR::Distribute(val_union *val,uint8_t hashed, index_item * idx_itm, off_t offset)
{
    idx_itm->table[hashed].value2data[idx_itm->table[hashed].count].offset=offset;
    memcpy(&idx_itm->table[hashed].value2data[idx_itm->table[hashed].count].value,val,sizeof(val_union));
    idx_itm->table[hashed].count++;
}


void DBMGR::GetTribleTree(index_node *tree)
{
    tree[1].range_left=0;
    tree[1].range_middle=9;
    tree[1].range_right=18;
    tree[1].range_end=27;
    for(int i=2;i<TRIBLE_TREE_LENGTH;i++)
    {
        uint parent=(i+1)/3;
        if(parent*3-1==i)
        {
            tree[i].range_left=tree[parent].range_left;
            tree[i].range_end=tree[parent].range_middle;
        }
        else if(parent*3==i)
        {
            tree[i].range_left=tree[parent].range_middle;
            tree[i].range_end=tree[parent].range_right;
        }
        else if(parent*3+1==i)
        {
            tree[i].range_left=tree[parent].range_right;
            tree[i].range_end=tree[parent].range_end;
        }
        for(int j=0;j<3;j++)
        {
            uint8_t gap=(tree[i].range_end-tree[i].range_left)/3;
            tree[i].range_middle=tree[i].range_left+gap;
            tree[i].range_right=tree[i].range_middle+gap;
        }
    }
    for(int i=1;i<14;i++)
    {
        tree[i].lchild=i*3-1;
        tree[i].mchild=i*3;
        tree[i].rchild=i*3+1;
    }
    for(int i=14;i<41;i++)
    {
        tree[i].lchild=-1;
        tree[i].mchild=-1;
        tree[i].rchild=-1;
    }
}


// private end
