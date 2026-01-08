## functions
```c
// 最多拷贝 num 字节， 遇到 \0 就终止了 
char * strncpy ( char * destination, const char * source, size_t num ); 

// 拷贝 num 字节 
void * memcpy ( void * destination, const void * source, size_t num );

// 比较
int strncmp ( const char * str1, const char * str2, size_t num ); 
int memcmp ( const void * ptr1, const void * ptr2, size_t num ); 
int strncasecmp( const char * str1, const char * str2, size_t num );


// 查找
char * strstr ( const char * str1, const char * str2 );       
char * strchr ( const char * str, int character ); 


// A sequence of calls to this function split str into tokens
char * strtok ( char * str, const char * delimiters ); 
#include <stdio.h> 
#include <string.h> 
int main () {   
	char str[] ="- This, a sample string.";   
	char * pch;   
	printf ("Splitting string \"%s\" into tokens:\n",str);   
	pch = strtok (str," ,.-");   
	while (pch != NULL)   {     
		// This |  a | sample | string
		printf ("%s\n",pch);     
		pch = strtok (NULL, " ,.-");   
	}   
	return 0; 
}

// strtok 的一份实现：找到第一个非分隔符，找到下一个分隔符置为\0
char *strtok(char *str, const char *delim) {
    static char *last_token = NULL;  
    if (str) {
        last_token = str;
    } else if (!last_token) {  // 如果没有要处理的字符串且静态指针为空，则返回 NULL
        return NULL;
    }

    char *token_start = last_token;

    // 找到下一个分隔符出现的位置
    while (*last_token && strchr(delim, *last_token)) {
        ++last_token;
    }
    if (!*last_token) {  // 没有找到分隔符，返回 NULL
        last_token = NULL;
        return NULL;
    }
    token_start = last_token;

    // 找到子串结束的位置，并将其替换成 '\0' 字符
    while (*last_token && !strchr(delim, *last_token)) {
        ++last_token;
    }
    if (*last_token) {
        *last_token++ = '\0';  // 将分隔符替换成 '\0' 字符
    } else {
        last_token = NULL;
    }

    return token_start;
}

// scan str1 for the first occurrence of any character in the string str2, return the pointer
// if it fails, return NULL
char * strpbrk ( const char *str1, const char *str2); 
#include <stdio.h> 
#include <string.h> 
int main () {   
	char str[] = "fcba73";   
	char keys[] = "1234567890";     
	char *p = strpbrk (str,keys);   
	printf ("The first number in str is at position %x.\n",p);   
	return 0; 
}


// 生成随机数
// srand(seed) 是一个随机数生成器，同一个随机数生成器，随机数序列是确定的
// 通常用时间做种子，srand(time(NULL)),
// rand() 产生一个[0,RAND_MAX]整数
// 实际项目用这个函数不够安全
srand(time(NULL)); 
for (int i = 0; i < 10; ++i) {     
	cout << rand() << endl; 
}
```


## backtrace
```C
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BT_BUFSIZE 100

void sig_get_backtrace(int signo)
{
	int nptrs;
	void *buffer[BT_BUFSIZE];
	char **strings;

	nptrs = backtrace(buffer, BT_BUFSIZE);
	printf("backtrace returns %d addresses\n", nptrs);

	strings = backtrace_symbols(buffer, nptrs);
	if (NULL == strings) {
		perror("backtrace_symbols");
		exit(-1);
	}

	for (int i = 0; i < nptrs; ++i)
		printf("%s\n", strings[i]);

	signal(signo, SIG_DFL);
}
```