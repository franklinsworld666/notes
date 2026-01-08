---
title: bash 语法
tags:  
notebook: 1101 程序设计 
---

[TOC]

## 调试
调试的关键在3个问题：
+ 常见的错误
+ 用什么工具自动检查语法错误
+ 失败时，能立即告诉我
+ 可以打印脚本执行的全过程

### 常见的错误
编写 Shell 脚本的时候，一定要考虑到命令失败的情况，否则很容易出错。

```shell
#! /bin/bash 

# 下面这句就没有考虑命令失败的情况
# 如果 dir_name 不存在，就会删除 ～ 目录下的全部文件
cd $dir_name && rm *

# 正确的写法
[ -d $dir_name ] && cd $dir_name && rm *
```

### 语法检查工具
+ 在线语法检查 [shellcheck](https://www.shellcheck.net/)
+ 命令行使用，需要安装 shellcheck 工具

### 利用 set 命令调试
```shell
#!/bin/bash
# 开头加上下面一句
# -e 遇到失败，停止执行
# -u 遇到不存在的变量就会报错，并停止执行
# -x 打印执行语句
# -o 只要一个子命令失败，管道命令就失败
set -euxo pipefail

```


## shell 运行环境
### 登录 Session 初始化
登录 Session 一般进行整个系统环境的初始化，启动的初始化脚本依次如下。

+ `/etc/profile`：所有用户的全局配置脚本。
+ `/etc/profile.d`: 目录里面所有.sh文件
+ `~/.bash_profile`：用户的个人配置脚本。如果该脚本存在，则执行完就不再往下执行。
+ `~/.bash_login`：如果`~/.bash_profile`没找到，则尝试执行这个脚本（C shell 的初始化脚本）。如果该脚本存在，则执行完就不再往下执行。
+ `~/.profile`：如果`~/.bash_profile`和`~/.bash_login`都没找到，则尝试读取这个脚本（Bourne shell 和 Korn shell 的初始化脚本）。

Linux 发行版更新的时候，会更新`/etc`里面的文件，比如`/etc/profile`，因此不要直接修改这个文件。如果想修改所有用户的登陆环境，就在`/etc/profile.d`目录里面新建`.sh`脚本。

如果想修改你个人的登录环境，一般是写在`~/.bash_profile`里面。


### shell 启动的时候获得哪些变量
shell 启动的时候会加载对应的 `profile` 文件，获得文件定义的环境变量。

环境变量：对所有 shell 进程都可见的变量。 查看环境变量：
```
env
```

自定义变量：只有在当前 shell 进程才能查看的变量。
```shell
# 下面的结果里包括了环境环量
set
```

### 子 shell 从父 shell 获取哪些变量
+ 子 shell 会拷贝父 shell 的环境变量
+ 子 shell 无法获得父 shell 的自定义变量
+ 子 shell 取消或修改环境变量，对父 shell 没有影响 


## 变量
### 自定义变量
+ 由字母、数字、下划线组成
+ 区分大小写，不超过20个字符
+ 变量赋值，等号与值之间不要有空格
+ 使用变量的值的时候，要加 `$` 或者 `${}`
+ 如果变量中间有空格，要使用 "$param" 的方式，否则可能命令执行出错
+ 自定义变量只在这个进程中起作用

```shell
#!/bin/bash
#变量赋值，等号两边不能有空格
parameter1="hello, world"
p2=$(ls -l ~)

# 使用变量要加$, 没有的话认为是普通字符串：
echo $parameter1
echo ${parameter1}

# 取消变量，unset 或者 设置为空字符串
unset parameter1
parameter1=''

# 把局部环境变量变成全局，对子 shell 可见
export parameter1
```

### 特殊变量
+ `$0` 代表脚本名
+ `$1` `${10}`  等等，代表脚本的第1个、第10个输入参数
+ `$#`  代表脚本名称后面跟的参数个数
+ `$@`  代表了所有参数，并且可以被遍历
+ `$*`  代表了所有参数，并且作为一个整体
+ `$$`  代表当前脚本程序的进程ID
+ `$?`  代表上一条命令的退出状态

关于 `$@` 和 `$*` 的例子：
```shell
!/bin/bash
set -exu

count=1
for param in "$*"
do
    echo "\$* parameter #$count = $param"
    count=$[ $count + 1 ]
done


count=1
for param in "$@"
do
    echo "\$@ parameter #$count = $param"
    count=$[ $count + 1 ]
done
```
结果：
```
./test.sh p1 p2 p3
$* parameter #1 = p1 p2 p3
$@ parameter #1 = p1
$@ parameter #2 = p2
$@ parameter #3 = p3
```


### 数组变量
```shell
#!/bin/bash
arr=(1 3 5 7 9)
$arr
${arr[0]}
${arr[*]}

doc=(*.md)
${doc[*]}
```
结果：
```shell
1
1
1 3 5 7 9

bash.md git.md
```


### 模式扩展
+ `?` : 任意单个字符，不包括空格
+ `*` : 任意数量的任意字符，可以匹配空字符
+ `[ab]` : a或b
+ `[a-d]` : a 到 d 中的一个
+ `g{a,b,c,d}e` : 分别扩展成 gae gbe gce gde
+ `g{a..d}e` : 分别扩展成 gae gbe gce gde
+ `$()` : 子命令扩展, `()` 内的命令会启动一个子 shell
+ `$(( 2 + 2 ))` : 算术扩展


### 转义
```shell
# 单引号，各种特殊字符在单引号里面，都会变为普通字符
echo '$USRE'

# 双引号，大部分特殊字符在双引号里面，变成普通字符。
# $ \ 反引号，这三个保持特殊含义
echo "$USER"
ehco "I'd say: \"hello\"  \\ "

# 文件名中有空格, 可以加双引号
ls "a b.txt"

# 双引号，还可以保持原始命令的输出格式
echo $(cal)
echo "$(cal)"

```

## 运算
### 赋值
```shell
#!/bin/bash
set -exu

value1=10

# 从其他变量赋值
value2=$value1
echo $value2

# 从命令结果赋值，推荐 $()
value3=`date +%y%m%d`
value4=$(date +%y%m%d)
echo $value3
echo $value4
```

### 算术运算
+ 整数算术运算：(())
+ 浮点运算：采用 bc

```shell
!/bin/bash
set -exu

# + - * / % ** ++ --
((1+2))
((3-2))
((2-2)) # $? 为1 ，代表失败
((2*3))
((5/2)) # 2
((5%2)) # 1
((2**3)) # 8
((a++))
((b--))



var1=10.46
var2=43.67
var3=$(bc << EOF
scale = 4
value4 = ($var1 * $var2)
value4
EOF
)
echo $var3
```
结果：
```
456.7882
```

### 16进制
```shell
# 算术表达式中，默认十进制，
var=$((0xff))
echo $var
```

### 位运算
```shell
# >> <<  & | ~ ^
echo $((16>>2))

echo $((17&3))

echo $((17|3))

```

### 逻辑运算
```shell
# < > <= >= ==  != && || !
# expr1?expr2:expr3

echo $((3 > 2))
echo $(( (3 > 2) || (4 <= 1) ))
echo $((a<1 ? 1 : 0))
```

## 条件判断
### if 语句
+ 基本语法：
```
# 测试条件：字符串测试、文件测试、数值测试
if [ condition ]; then
    echo "match condition"
fi

# 下面这种，还支持正则
if [[ condition ]];then

fi

# 复合条件
# && || !
if [ condition1 ] && [ condition2 ]; then
    echo "match condition"
fi
```

+ 字符串测试：
```shell
# 变量要放在双引号之中, 避免变量为空

# 字符串相等
if [ "$str" = "$str2" ]
if [ "$str" != "$str2" ]

# 字符串空
if [ -z "$stats" ];then
    echo "empty"
fi

# 字符串非空
if [ -n "$stats" ]

# 字符串比较大小
# 大于、小于符号要转义
if [ "$var1" \> "nihao" ];then
    echo " large "
else
    echo " small "
fi

```

+ 利用正则测试字符串
```shell
#!/bin/bash
set -exu
# 在 双方括号中，> 不用转义了；== 代表模式匹配
if [[ "hello" > "nihao" ]];then
    echo "large"
elif [[ testuser == *tu* ]];then
    echo "match"
fi
```
结果：
```
match
```

+ 文件测试
```shell
# 是否存在并且是文件
if [ -f file ]

# 是否是目录
if [ -d file ]

# 是否存在
if [ -e file ]

# 是否并且非空
if [ -s file ]

# 是否存在，并且可读、可写、可执行
if [ -r file ]
if [ -w file ]
if [ -x file ]

# 文件存在，并属于当前用户
if [ -O file ]

# file1 older than file2
# file1 newer than file2
if [ file1 -ot file2 ]
if [ file1 -nt file2 ]
```

+ 数值测试
```shell
# equal
if [ n1 -eq n2];then

# greater than or equal
if [ n1 -ge n2];then

-gt 
-le 
-lt 
-ne not equal 

# 数学运算


```

+ 用双括号测试数学表达式
```shell
#!/bin/bash
set -exu

var1=10
# 双括号里，不用转义了
if (( $var1 ** 2 > 90 ));then
    echo $var1
fi

# 双括号可以在 if 语句用，也可以在普通命令里用
# val++
# val--
# && || !
# & |  ~
# >>  << 
# 幂运算 ** 
(( val = val ** 2 ))
```
结果：
```
100
```

### case 语句
```shell
#!/bin/bash
set -exu

var1=test
# 下面进行模式匹配
case $var1 in
rich | barbara)
    echo "Welcome, $USER";;
test*)
    echo "Special testing";;
*)
    echo "Sorry, you are not allowed";;
esac
```
结果：
```
Special testing
```

## 循环
### for 循环
```shell
!/bin/bash
set -exu

list="Alabama Alaska Arizona"
for var in $list
do
    echo "visit $var"
done
```

C style for loop:
```shell
for (( a = 1; a < 10; a++ ))
do
    echo "The next number is $a"
done
```

### while 循环
```shell
while [ $var1 -ge 0 ]
do
    echo $var1
    var1=$(($var1 - 1))
done
```

### until 循环
```shell
var1=10
until [ $var1 -eq 0 ]
do 
    echo $var1
    var1=$(($var1 - 1))
done
```

### shift 操作
可以简单理解成，把输入的命令行参数向左移动，移出去的就永远丢了， `$1` 指向了新的参数。
```shell
#!/bin/bash
set -exu

while [ -n "$1" ]
do
    case "$1" in
    -a) echo "Found -a opt";;
    -b) param=$2
        echo "Found -b opt with param $2"
        shift 2;;
    --) shift
        break ;;
    *) echo "$1 is not an opt"
    esac
    shift
done
```
结果：
```
./test.sh -a -b test1 -d
Found -a opt
Found -b opt with param test1
```

### getopts 操作
`getopts` 操作，通常与while循环一起使用，取出脚本所有的带有前置连词线（`-`）的参数。只要遇到不带连词线的参数，getopts就会执行失败，从而退出循环。

```shell
!/bin/bash
# set -exu

while getopts 'ab:c' opt;
do
    case $opt in
    a) echo "Found -a opt";;
    b) echo "Found -b opt with param $OPTARG";;
    c) echo "Found -c opt";;
    *) echo "$opt is not an opt";;
    esac
done

# OPTIND 初值1，每处理一个就加1，结束的时候意味着连词线参数全部处理完毕
shift $(($OPTIND -1))
echo $1
```
结果：
```
./test.sh -ab test1 -c test2 test3
Found -a opt
Found -b opt with param test1
Found -c opt
test2
```


## 函数
主要关注3个问题：
+ 函数如何定义
+ 如何向函数传递参数
+ 函数如何返回结果
+ 如何使用其他文件中的函数

```shell
function myfunc() 
{
    # return 只能返回 0-255
    # 如果没有 return，返回值就是最后一条命令的执行结果，0成功，非0失败
    return 0
}

function myfunc2()
{
    local var="hello,world"
    # echo 输出打印，外部可以获取到
    # 函数的返回值，最后一条命令的执行结果，0成功，非0失败
    echo $var
}

par=$(myfunc2)
echo "$par"
```

向函数传参：
```shell
# 类似于在命令行传参
myfunc p1 p2
```

使用其他文件的函数：
```shell
# source other files, then you can use the functions defined in the files
source ./myfunc.sh

# another way
. ./myfunc.sh
```

## 输入
### 从命令行参数输入
```shell
./test.sh  2 5 "Rich Blue"
```

### 从文件输入
```shell
# redirect
wc -c < inputfile

# redirect to file descriptor
0< inputfile

# file redirect
filename='/etc/hosts'
while read myline
do
  echo "$myline"
done < $filename
```

### 从管道输入
```shell
date | wc -c 
```
另一例子：
```shell
count=1
cat test | while read line
do
    echo "Line $count: $line"
    count=$(($count + 1))
done
```

### 从控制台输入
```shell
# -p 参数指定用户输入的提示信息
read -p "Enter your name: " name
echo "Hello $name, Welcome"

# 可以输入多个参数
read -p "Enter your firstname and lastname:" firstname lastname
echo "Hi, $firstname $lastname"

# -t 设置了超时的秒数
read -t 3 -p "enter in 3 secs:" name

# -n 指定读取若干个字符
read -n 3 letter

# -s 用户输入不显示在屏幕上，如：隐藏密码
read -s passwd
```

## 输出
### 输出到控制台
```shell
#!/bin/bash
echo "Hello, world"
echo $HOME
```

### 输出到文件
```shell
# 利用重定位输出到文件
touch file1
date > file1
date >> file1

# only redirct error msgs
ls -al badfile 2> test4

# redirect stderr & stdout
ls -al okfile badfile &> test4

# redirect to the file desciptor
echo "this is an error" >&2
```

### 创建临时文件
直接创建临时文件，尤其在 `/tmp` 目录里面，往往会导致安全问题.`mktemp` 命令就是为安全创建临时文件而设计的。

```shell
#!/bin/bash

# 为了保证退出时，删除临时文件
trap 'rm -f "$TMPFILE"' EXIT

# 如果临时文件创建失败，退出
TMPFILE=$(mktemp) || exit 1
echo "Our temp file is $TMPFILE"
```


## 其他


+ 注释
```shell
# 这是一行注释

<<EOF
这也可以
达到
多行注释
EOF
```

+ sed 
`sed` 流处理器，一次读一行并按照命令处理，把处理结果输出，并不改变文件本身。
```shell
# 基本语法
# -n： 不显示结果，silent
# -f： 指定要执行的脚本
# -e: 执行多个命令
# script 是单引号包括的文本字符串
sed option script dstfile

# p: 输出匹配的行
sed -n '2 p' data.txt
sed -n '3,4 p' data.txt
sed -n '2! p' data.txt
# 输出匹配 beijing 开头的行 
sed -n '/^beijing/ p' data.txt

# 替换行内第N个匹配的数据, 并只显示匹配的结果
sed -n 's/30/40/N p' data.txt
# 匹配模式涉及 / 时，可以用 # 分隔
sed -n 's#30#40 p' data.txt
# g 会处理行中的全部匹配，否则只处理第一个
sed -n 's/30/40/g p' data.txt


# 增加行
# 在第4行后，加一行
sed '4a li 33 english' data.txt
# 在每一行后，加一行
sed 'a li 33 english' data.txt
# 在第2行，第3行后面，都加一行
sed '2,3a li 33 english' data.txt
# 在最后一行后，加一行
sed '$a li 33 english' data.txt

# 增加行
# 在第2行前面，插入
sed '2i zhao 32 York' data.txt

# 删除行
sed '2 d' data.txt
sed '2,3 d' data.txt
# 删除每一行
sed 'd' data.txt

# 修改行
# 第2行修改为 hello
sed '2c hello' data.txt

# 执行多个命令，-e
sed -e 's/dog/cat/; s/brown/green/' data1.txt

# 匹配，并执行命令的语法 
sed '/pattern/command' dstfile
sed '/samantha/s/bash/csh/p' /etc/passwd
sed '/number1/d' data1
```

+ awk 语法

`awk` 就是把文件逐行读入，以空格默认分片，切开的部分再进行处理
```shell
# 基本语法
# F --- set separator
# v --- define a variable and default value
# $0: 整行
# $1: 第一个字段
# $NF: the total num of data fields
# $NR: the num of input records processed
# FS : input field separator
# OFS : output field separator
awk option script dstfile

# 分割符
# 可以设置多个分割符
awk -F: '{print $1}' data2.txt
awk -F "[ ;]" '{print $1}' data2.txt

# 多条命令
awk '{$4="Yao"; print $0}' data2.txt

# 显示的时候默认的分割符是空格，可以设置
awk '{print $1,$2}' data2.txt
awk -v OFS=',' '{print $1,$2}' data2.txt

# NR 代表处理的记录数
# 显示第一行
awk 'NR == 1 {print $0}' data2.txt
```

+ 屏蔽信号
```shell
trap "" INT QUIT
```

+ 参考资料
    + Bash 脚本教程
    + Linux Command Line and Shell Scripting Bible
    + Advanced Bash-Scripting Guide
    + linux shell脚本攻略
    + bash Pitfalls(BashPitfalls)