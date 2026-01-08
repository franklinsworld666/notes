---
title: IO 总结
tags: 
notebook: 1101 程序设计 
---

[TOC]

## 打开文件
### open
类型：系统调用  
适用范围：regular file, terminal, socket, pipe, FIFO, devices  

函数原型：
```C
// open an existing file or creat and open a new file
// flags: 文件访问权限，只读、只写、读写等
// mode: 如果新建一个文件，要设置 mode，类似执行 chmod
// Returns: Returns file descriptor on success, or –1 on error and set errno
#include <sys/stat.h>
#include <fcntl.h>
int open(const char *pathname, int flags, mode_t mode);
```

例子：
```C 
// 读一个存在的文件
inputFd = open(path, O_RDONLY);
if (-1 == inputFd)
    printf("open %s fail, errno:%d", path, errno);

// 写一个存在的文件
// 注意：默认从头开始写，会破坏原有数据
inputFd = open(path, O_WRONLY);

// 追加到一个存在的文件
inputFd = open(path, O_WRONLY | O_APPEND);

// 如果文件存在，就打开并截断
// 如果不存在，就新建一个文件
// 注意：需要设置 mode
openFlags = O_CREAT | O_WRONLY | O_TRUNC;

filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
S_IROTH | S_IWOTH; /* rw-rw-rw- */

outputFd = open(path, openFlags, filePerms);
```

### fopen
类型：标准库函数  
适用范围：regular file  

函数原型：
```C 
// 打开一个存在的文件，或新建并打开文件
// 标准 IO 有三种缓冲：全缓冲（文件）、行缓冲（标准输入）、无缓冲（标准错误）
// type: 设置流的读写方式
// r: O_RDONLY
// w: O_WRONLY | O_TRUNC | O_CREAT
// a: O_WRONLY | O_APPEND | O_CREAT
// r+: O_RDWR
// w+: O_RDWR | O_TRUNC | O_CREAT
// a+: O_RDWR | O_APPEND | O_CREAT
// 注意：如果打开二进制文件，后面加 b，如 rb 
// 注意：本函数无法设置权限，只能通过调整 umask 来限制权限
FILE *fopen(const char *path, const char *type);

// 用于 fopen 无法打开的特殊文件，即先用专用函数打开特殊文件，然后把它和标准 IO 流关联
FILE *fdopen(int fd, const char *type);
```

例子：
```C 
// 读一个文件
// 文件若不存在，返回 NULL
FILE *f = fopen("./txt", "r");

// 写一个文件
// 文件存在，截断为0
// 文件不存在，就创建
FILE *f = fopen("./txt", "w");

// 追加到一个文件
// 文件存在，在末尾追加
// 文件不存在，创建
FILE *f = fopen("./txt", "a");
```

## 读一定字节
### read
类型：系统调用  
适用范围：regular file, terminal, socket, pipe, FIFO, devices  
阻塞与非阻塞：都支持，但两种情况返回值一致，不需要特殊处理

函数原型：
```C 
// 成功，返回读到的字节数
// 文件尾，返回0
// 失败，返回 -1，and set errno
// 注意：非阻塞读，如果缓冲区没有数据，这时返回 -1， 并设置 errno 为 EAGAIN
// 注意：read 不会在结尾自动加 \0
ssize_t read(int fd, void *buffer, size_t count);
```

例子：
```C 
char buffer[MAX_READ + 1];
ssize_t numRead;
numRead = read(STDIN_FILENO, buffer, MAX_READ);
if (numRead == -1)
    errExit("read");

buffer[numRead] = '\0';
printf("The input data was: %s\n", buffer);
```

### fread
类型：标准库函数  
适用：普通文件  
阻塞与非阻塞：读普通文件，是不会阻塞的，要么读到，要么读不到

函数原型：
```C 
// 从 stream 最多读 blknum * size 字节，存放在 ptr
// 成功，返回blknum
// 出错，返回 0 或者 小于 blknum，需要通过 feof 和 ferror 来判断
size_t fread(void *ptr, size_t size, size_t blknum, FILE *stream);
```

例子：
```C 
// 读出一个文件的内容
pFile = fopen("myfile.bin", "rb");
if (NULL == pFile)
    return 0;

// obtain file size
fseek(pFile, 0, SEEK_END);
lSize = ftell(pFile);
rewind(pFile);

// buf is large enough
result = fread(buf, 1, lSize, pFile);
if (lSize != result) {
    printf("Reading error\n");
    return 0;
}
fclose(pFile);
```

### pread
类型：系统调用  
适用范围：regular file, terminal, socket, pipe, FIFO, devices   
阻塞与非阻塞：都支持，类似 read

函数原型：
```C 
// 类似 read，但在函数指定的位置开始读，此调用不改变文件的 offset
// 适用场景：多线程场景下，lseek 和 read 之间可能被其他线程插一脚，
ssize_t pread(int fd, void *buffer, size_t count, off_t offset);
```

## 读一个字节
### fgetc
### getchar
类型：标准库函数  
适用范围：普通文件  
阻塞和非阻塞：读写普通文件都是非阻塞

函数原型：
```C 
// 成功，返回下一个字符
// 文件尾，返回 EOF
// 出错，返回 EOF
int fgetc(FILE *fp);

// 返回值同上
int getchar(void);

// 判断出错 或 文件结尾
// 条件为真，返回非 0
// 条件为假，返回 0
int ferror(FILE *fp);
int feof(FILE *fp);
```

## 读一行
### fgets
类型：标准库函数  
适用范围：普通文件, 终端   
阻塞和非阻塞：读写普通文件都是非阻塞

函数原型：
```C 
// 从指定的流读取数据，直到换行符前一个字符，或者 num-1 个字符，缓冲区最后一个是 null
// 成功，返回 str
// 文件尾，返回NULL
// 出错，返回NULL，具体要通过 feof 和 ferror 确认
// 注意：可能无法读满一行
char *fgets (char *str, int num, FILE *stream );
```

实例：
```C 
#include <stdio.h>

int main()
{
   FILE * pFile;
   char mystring [100] = {0};

   pFile = fopen ("myfile.txt" , "r");
   if (pFile == NULL) 
       perror ("Error opening file");
   else {
     if ( fgets (mystring , 100 , pFile) != NULL )
       fputs (mystring, stdout);
     fclose (pFile);
   }
   return 0;
}
```

## 写一定字节
### write
类型：系统调用  
适用范围：regular file, terminal, socket, pipe, FIFO, devices   

函数原型：
```C 
// 成功，返回写入的字节数
// 失败，返回 -1，and set errno
// 注意：非阻塞写，如果缓冲区没有空间，这时返回 -1， 并设置 errno 为 EAGAIN
// 注意：成功，表示内容进了缓冲区，内核会异步把缓冲区写入磁盘
ssize_t write(int fd, void *buffer, size_t count);
```

### fwrite
类型：标准库函数  
适用：普通文件  
阻塞与非阻塞：读普通文件，是不会阻塞的，要么读到，要么读不到

函数原型：
```C 
// 写 blknum * size 字节
// 成功，返回blknum
// 出错，返回 0 或者 小于 blknum，需要通过 ferror 来判断
size_t fwrite (const void *ptr, size_t size, size_t blknum, FILE *stream );
```

例子：
```C 
#include <stdio.h>

int main ()
{
  FILE * pFile;
  char buffer[] = { 'x' , 'y' , 'z' };
  pFile = fopen ("myfile.bin", "wb");
  fwrite (buffer , sizeof(char), 3, pFile);
  fclose (pFile);
  return 0;
}
```

### pwrite
类型：系统调用  
适用范围：regular file, terminal, socket, pipe, FIFO, devices  

函数原型：
```C 
// 类似 write，但在函数指定的位置开始读，此调用不改变文件的 offset
// 适用场景：多线程场景下，lseek 和 write 之间可能被其他线程插一脚，
ssize_t pwrite(int fd, void *buffer, size_t count, off_t offset);
```

## 写一个字节
### fputc
### putchar
类型：标准库函数  
适用范围：普通文件  
阻塞和非阻塞：读写普通文件都是非阻塞

函数原型：
```C 
// 成功，返回 c
// 出错，返回 EOF
int fputc(int c, FILE *fp);

// 返回值同上
int putchar(int c);
```

## 写一行
### fputs
类型：标准库函数  
适用范围：普通文件, 终端  
阻塞和非阻塞：读写普通文件都是非阻塞

函数原型：
```C 
// 把 str 字符串输出
// 成功，返回 non-negative value
// 失败，返回 EOF
// 注意：str 并不一定需要有换行符，如果你要就自己加, 函数不会自动帮你加
int fputs ( const char * str, FILE * stream );
```

## 格式化输入输出
类型：标准函数库  
适用：普通文件、终端  

函数原型：
```C 
//  输出格式化数据到 stdout
int printf ( const char *format, ... );

// 输出格式化数据到流 stream
int fprintf ( FILE *stream, const char *format, ... );

// 输出格式化数据到字符串 s，最多接收 n-1 个字符，其余会被抛弃
// 注意：该函数会在末尾自动加 null，但为了不必记住还是手动加
int snprintf ( char *s, size_t n, const char *format, ... );

// 从标准输入读格式化数据
int scanf ( const char *format, ... ); 

// 从流读格式化数据
int fscanf (FILE *stream, const char *format, ... );

// 从字符串读格式化数据
int sscanf (const char *s, const char *format, ...);

// 和 printf 相比，参数变了而已
// 从可变参数获取数据，输出到标准输出
int vprintf ( const char *format, va_list arg);

// 从可变参数获取数据，输出到流
int vfprintf (FILE *stream, const char *format, va_list arg);

// 从可变参数获取数据，输出到字符串
// 注意：最多 n-1 个字符，多余的会丢弃
int vsnprintf (char *s, size_t n, const char *format, va_list arg);

// 从标准输入读数据到 可变参数
int vscanf ( const char * format, va_list arg );

// 从流读数据到 可变参数
int vfscanf ( FILE * stream, const char * format, va_list arg );

// 从字符串读数据到 可变参数
int vsscanf ( const char * s, const char * format, va_list arg );
```

例子：
```C 
/* snprintf example */
#include <stdio.h>

int main ()
{
  char buffer [100] = {0};
  int cx;

  cx = snprintf ( buffer, 100, "The half of %d is %d", 60, 60/2 );

  if (cx>=0 && cx<100) 
    snprintf ( buffer+cx, 100-cx, ", and the half of that is %d.", 60/2/2 );

  fputs (buffer,stdout);
  return 0;
}
```

例子：
```C 
/* sscanf example */
#include <stdio.h>

int main ()
{
  char sentence []="Rudolph is 12 years old";
  char str [20];
  int i;

  sscanf (sentence,"%s %*s %d",str,&i);
  printf ("%s -> %d\n",str,i);
  
  return 0;
}
```

例子：
```C 
/* vfprintf example */
#include <stdio.h>
#include <stdarg.h>

void WriteFormatted (FILE * stream, const char * format, ...)
{
  va_list args;
  va_start (args, format);
  vfprintf (stream, format, args);
  va_end (args);
}

int main ()
{
   FILE * pFile;

   pFile = fopen ("myfile.txt","w");

   WriteFormatted (pFile,"Call with %d variable argument.\n",1);
   WriteFormatted (pFile,"Call with %d variable %s.\n",2,"arguments");

   fclose (pFile);

   return 0;
}
```

例子：
```C 
/* vsscanf example */
#include <stdio.h>
#include <stdarg.h>

void GetMatches ( const char * str, const char * format, ... )
{
  va_list args;
  va_start (args, format);
  vsscanf (str, format, args);
  va_end (args);
}

int main ()
{
  int val;
  char buf[100];

  GetMatches ( "99 bottles of beer on the wall", " %d %s ", &val, buf);

  printf ("Product: %s\nQuantity: %d\n", buf, val);

  return 0;
}
```

## 读写网络数据
+ TCP 读：read readv 
+ TCP 写：write writev
+ UDP 读：recv recvfrom recvmsg recvmmsg
+ UDP 写：send sendto sendmsg sendmmsg

### read
```C 
#include <unistd.h>

// 成功，返回读到的字节数
// 文件尾，返回0
// 失败，返回 -1，and set errno
// 注意：非阻塞读，如果缓冲区没有数据，这时返回 -1， EAGAIN
// 注意：read 不会在结尾自动加 \0
ssize_t read(int fd, void *buffer, size_t count);
```

### recv and recvfrom
```C 
#include <sys/types.h>
#include <sys/socket.h>

// 从 sockfd 接收数据，最多 len 个字节
// 成功，返回收到的字节数，可能小于 len
// 对方关闭，返回 0
// 失败，返回 -1
// 阻塞：缓冲区无数据，阻塞在那里
// 非阻塞：缓冲区无数据，返回-1，EAGAIN
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

// 和 recv 的区别是，可以获取源地址
// addrlen 是一个 value-result argument，告知地址缓冲区大小 
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
struct sockaddr *src_addr, 
socklen_t *addrlen);
```

### recvmsg
```C 
// 成功，返回收到的字节数
// 对方关闭，返回 0
// 失败，返回 -1
// 阻塞：缓冲区无数据，阻塞在那里
// 非阻塞：缓冲区无数据，返回-1，EAGAIN
// 对于 UDP： 是否会收到部分数据呢？只要确保用户缓冲区够大，就不会
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);

struct iovec {                    /* Scatter/gather array items */
    void  *iov_base;              /* Starting address */
    size_t iov_len;               /* Number of bytes to transfer */
};

// msg_name: udp 源地址
// msg_iovlen: iovec 的数量
// msg_control： 内核上传的辅助信息，比如 udp 的目的地址、目的端口等
// msg_controllen: 内核返回的辅助信息的长度
struct msghdr {
    void         *msg_name;       /* optional address */
    socklen_t     msg_namelen;    /* size of address */
    struct iovec *msg_iov;        /* scatter/gather array */
    size_t        msg_iovlen;     /* # elements in msg_iov */
    void         *msg_control;    /* ancillary data, see below */
    size_t        msg_controllen; /* ancillary data buffer len */
    int           msg_flags;      /* flags on received message */
};

// 辅助信息数据结构，只能通过特定的宏获取具体的内容
// 具体见 cmsg
struct cmsghdr *CMSG_FIRSTHDR(struct msghdr *msgh);
struct cmsghdr *CMSG_NXTHDR(struct msghdr *msgh, struct cmsghdr *cmsg);
size_t CMSG_ALIGN(size_t length);
size_t CMSG_SPACE(size_t length);
size_t CMSG_LEN(size_t length);
unsigned char *CMSG_DATA(struct cmsghdr *cmsg);

struct cmsghdr {
    socklen_t     cmsg_len;     /* data byte count, including hdr */
    int           cmsg_level;   /* originating protocol */
    int           cmsg_type;    /* protocol-specific type */
/* followed by
    unsigned char cmsg_data[]; */
};
```

### recvmmsg
```C 
#define _GNU_SOURCE
#include <sys/socket.h>

// 支持接收多个 msg，同时多了个超时功能
// msgvec： struct mmsghdr msgvec[vlen]
// 成功，返回收到的 msg 数
// 失败，返回 -1
// 阻塞：直到收到 vlen 个msg，或者超时
// 非阻塞：接收不超过 vlen 个msg，立即返回
int recvmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
    unsigned int flags, struct timespec *timeout);

// msg_len : 当前这个 msg 的字节数
struct mmsghdr {
    struct msghdr msg_hdr;  /* Message header */
    unsigned int  msg_len;  /* Number of received bytes for header */
};
```

### readv
```C 
#include <sys/uio.h>

// 分散读
// 成功：返回读到的字节数
// 失败：返回 -1
// 阻塞：阻塞直到读到数据，可能小于 buffer 空间
// 非阻塞：立即返回，读到多少是多少，读不到EAGAIN
// 对于 UDP： 一个 msg 的数据分散在不同地方
// 注意：buffer 是按照顺序被填满的
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

struct iovec {
    void  *iov_base;    /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};
```

例子：
```C 
char *str0 = "hello ";
char *str1 = "world\n";
struct iovec iov[2];
ssize_t nwritten;

iov[0].iov_base = str0;
iov[0].iov_len = strlen(str0);
iov[1].iov_base = str1;
iov[1].iov_len = strlen(str1);

nwritten = writev(STDOUT_FILENO, iov, 2);
```

### write
```C 
// 成功，返回写入的字节数
// 失败，返回 -1，and set errno
// 阻塞：缓冲区满，一直阻塞
// 非阻塞，如果缓冲区没有空间，这时返回 -1， EAGAIN
// 注意：成功，表示内容进了缓冲区，并不表示发送出去了
ssize_t write(int fd, void *buffer, size_t count);
```

### send and sendto
```C 
#include <sys/types.h>
#include <sys/socket.h>

// 成功，返回发送的字节数
// 失败，返回-1，
// 阻塞：如果缓冲区无空间，阻塞
// 非阻塞：如果缓冲无空间，返回-1，EAGAIN
ssize_t send(int sockfd, const void *buf, size_t len, int flags);

// 只是多了一个目的地址
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen);
```

### sendmsg
```C 
#include <sys/types.h>
#include <sys/socket.h>

// 成功，返回发送的字节数
// 失败，返回 -1 
// 阻塞：缓冲区无空间，一直阻塞
// 非阻塞：缓冲区无空间，立即返回 -1， EAGAIN
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);

// msg_name : 可以指定目的地址
// msg_control： 可以填充你的控制信息，上限由 /proc/sys/net/core/optmem_max 决定
struct msghdr {
    void         *msg_name;       /* optional address */
    socklen_t     msg_namelen;    /* size of address */
    struct iovec *msg_iov;        /* scatter/gather array */
    size_t        msg_iovlen;     /* # elements in msg_iov */
    void         *msg_control;    /* ancillary data, see below */
    size_t        msg_controllen; /* ancillary data buffer len */
    int           msg_flags;      /* flags on received message */
};
```

### sendmmsg
```C 
#define _GNU_SOURCE
#include <sys/socket.h>

// 成功，返回发出的 msg 数
// 失败，返回 -1
// 阻塞：一直到发出 vlen 个 msg
// 非阻塞：尽可能发送，立即返回
int sendmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
unsigned int flags);

// msg_len： 这个 msg 的长度
struct mmsghdr {
    struct msghdr msg_hdr;  /* Message header */
    unsigned int  msg_len;  /* Number of bytes transmitted */
};
```

### writev
```C 
#include <sys/uio.h>

// 集中写
// 成功，返回发送的字节数，可能无法全部发出
// 失败，返回 -1 
// 阻塞：缓冲区无空间，一直阻塞
// 非阻塞：立即返回 -1， EAGAIN
// 对于 UDP，全部数据按照一个 msg 去写, 当然还是不推荐这么用
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```

## 其他 
### 不同缓冲长度对 IO 效率的影响
例子：
```C 
// 将标准输入复制到标准输出
while ((n = read(STDIN_FILENO, buf, BUFSIZE)) > 0)
    if (write(STDOUT_FILENO, buf, n) != n)
        printf("write error\n");
```
结论：
+ 在磁盘块大小4096左右，程序的 CPU 时间最小
+ 大多数文件系统采取 预读 技术，从 32 字节开始，程序的时钟时间就变化不大了
+ 我如何选择 BUFSIZE，32 ～ 4096 其实都行
+ 更大的 BUFSIZE 可以减少 system call, 但是主要的影响因素已经不再是系统调用次数，而是数据拷贝

### 标准 IO 的效率
```C
#include <stdio.h>

int main()
{
    char buf[MAXLINE];
    while (fgets(buf, MAXLINE, stdin) != NULL)
        if (fputs(buf, stdout) == EOF)
            perror("output error!");

    return 0;
}
```

结论（具体数据见 APUE 图5-6）：
+ 采用标准IO库，和使用系统函数调用read和write相比，并没有慢很多。对于复杂程序而言，主要的用户CPU时间是由应用处理消耗的。
+ 对比 CPU 时间，采用系统调用read和write，CPU 时间最少，因为系统调用次数更少，但差异并不大。
+ 对比用户时间，两种方式差异明显。
+ 时钟时间的差异，主要来自用户时间差异以及等待IO结束所消耗时间的差异。


### 同步写对 IO 性能的影响
### close
类型：系统调用   
适用范围：regular file, terminal, socket, pipe, FIFO, devices  

函数原型：
```C 
// 成功，返回0
// 失败，返回 -1，并设置 errno
// 文件关闭时，还会释放文件记录锁
int close (int fd);
```

### fclose
类型：标准库函数  
适用：普通文件

函数原型：
```C 
// 文件关闭的时候，会自动 flush 输出缓冲区，丢弃全部输入缓冲区，并释放分配的缓冲区空间
int fclose(FILE *fp);
```

### sync && fsync
类型：系统调用  
适用：All

函数原型：
```C 
// 把修改过的块缓冲区数据，放入队列，然后就返回，并不等待写磁盘完成
void sync(void);

// 等待写磁盘完成
// 成功，返回0
// 失败，返回-1
int fsync(int fd);
```

Linux 系统默认是延迟写。  
同步写和延迟写相比，时间会增加很多。  

### 定位
+ lseek  
类型：系统调用  
适用：All

函数原型：
```C 
// 设置文件偏移量
// offset：相对基准的偏移量，基准可以选文件开头、当前位置、文件结尾
// 成功，返回新的文件偏移量
// 失败，-1
// 注意：每个进程在内核中，维护一张本进程的文件表，文件表项包含了文件的标志、文件偏移、i节点指针
// 注意：两个进程打开同一个文件，在内核中会有2个文件表项，即有不同的文件偏移量
off_t lseek(int fd, off_t offset, int whence);
```

例子：
```C 
// 获取当前偏移量
off_t curpos = lseek(fd, 0, SEEK_CUR);
if (-1 == curpos)
    printf("seek error, err:%d\n", errno);
```

+ 由于移植性原因，ftell 和 fseek 就不要用了

+ rewind  
函数原型：
```C 
// Set position of stream to the beginning
void rewind ( FILE * stream );
```

+ fgetpos and fsetpos  
类型：标准库函数  
适用：普通文件，终端

函数原型：
```C 
// Get current position in the stream
// Success, return 0
// fail, return non-zero
int fgetpos ( FILE * stream, fpos_t * pos );

// Set stream indicator of stream
// Success, return 0
// fail, return non-zero
int fsetpos ( FILE * stream, const fpos_t * pos );
```

例子：
```C 
/* fgetpos example */
#include <stdio.h>
int main ()
{
   FILE * pFile;
   int c;
   int n;
   fpos_t pos;

   pFile = fopen ("myfile.txt","r");
   if (pFile==NULL) perror ("Error opening file");
   else
   {
     c = fgetc (pFile);
     printf ("1st character is %c\n",c);
     fgetpos (pFile,&pos);
     for (n=0;n<3;n++)
     {
        fsetpos (pFile,&pos);
        c = fgetc (pFile);
        printf ("2nd character is %c\n",c);
     }
     fclose (pFile);
   }
   return 0;
}
```

### Direct IO
transfer data directly from user space to a file or disk device. However, for most applications, it can considerably degrade the performance. This is because the kernel applies a lot of optimizations to improve the performance of IO. All of these optimizations are lost when using direct IO
