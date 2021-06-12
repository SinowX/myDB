#include<stdio.h>
#include<stdlib.h>
#include<string.h>

namespace Algor{

// KMP 字符串匹配算法——获取部分匹配表
void GetPartMatchTB(char * str,int arr[])
{
    int len=strlen(str);
    char * substr=(char *)malloc(len*sizeof(char));
    char * prefix=(char *)malloc(len*sizeof(char));
    char * suffix=(char *)malloc(len*sizeof(char));

    // for(int i=0;i<len;i++) First Round Can be Ignored
    for(int i=1;i<len;i++)
    {
        // printf("ROUND %d\n",i);
        strncpy(substr,str,i+1);
        substr[i+1]='\0';
        for(int j=0;j<i;j++)
        {
            strncpy(prefix,substr,j+1);
            prefix[j+1]='\0';
            strncpy(suffix,&substr[strlen(substr)-1-j],j+1);
            suffix[j+1]='\0';

            // printf("PRE: %s SUF: %s\n",prefix,suffix);
            if(!strcmp(prefix,suffix))
            {
                if(strlen(prefix)>arr[i])
                    arr[i]=strlen(prefix);
            }
        }

    }
}

int KMP(char *srcstr, char *substr)
{
    int total_match=0;


    int srclen=strlen(srcstr);
    int sublen=strlen(substr);
    int part_match[sublen]={0};
    
    GetPartMatchTB(substr,part_match);
    
    int cur_index=0;
    int match_cnt=0;
    while(cur_index+sublen<=srclen)
    {
        if(substr[match_cnt]==srcstr[cur_index+match_cnt])
        {
            match_cnt++;
            if(match_cnt==sublen)
            {
                total_match++;
                // printf("Match at: %d\n",cur_index);
            }
        }else{
            if(match_cnt==0)
            {
                cur_index+=1;
            }else{
                cur_index+=match_cnt-part_match[match_cnt-1];
            }
            match_cnt=0;
        }
    }

    return total_match;
}


}


namespace Hash{
//返回一个无符号整型值
unsigned int Murmur(const void *key, int len)
{
    const unsigned int m = 0x5bd1e995;

    const int r = 24;
    const int seed = 97;
    unsigned int h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4)
    {
        unsigned int k = *(unsigned int *)data;
        k *= m; 
        k ^= k >> r; 
        k *= m; 
        h *= m; 
        h ^= k;
        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array
    switch(len)
    {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
            h *= m;
    };
    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}
}