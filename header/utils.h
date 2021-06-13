#ifndef __UTILS
#define __UTILS



namespace Algor{

// KMP 字符串匹配算法——获取部分匹配表
void GetPartMatchTB(char * str,int arr[]);

int KMP(char *srcstr, char *substr);

}


namespace Hash{
//返回一个无符号整型值
unsigned int Murmur(const void *key, int len);
}

#endif