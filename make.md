[TOC]


## Makefile的目标
1. 如果工程没有编绎过，那么所有c文件都要编绎并链接;
2. 如果有一个c文件改变了，那么就只编绎这个改过的文件, 并链接;
3. 如果头文件发生改变， 那么编绎所有包含这个头文件的文件，并链接目标程序;
4. 使用 makefile 模版，开始新的项目时，只用稍作修改即可；


## 编绎和链接
### 生成目标程序的主要步骤
```bash
# 预处理，可查看 test.ii 
gcc -E test.c > test.ii   

# 编译成汇编代码
gcc -S test.ii

# 汇编代码 变成 机器码
gcc -c test.s 

# 链接
gcc test.o -o test
```

### gcc 的参数
```C
// 优化等级，默认-O2，但更高的等级不一定更高的效率
-O[0,1,2]

// 定义代码中的宏, 如果中括号缺省则默认值为1
-Dname[=definition]

// 添加搜索头文件的目录
-Idir

// 打开告警, 把告警作为错误处理
-Wall -Wno-deprecated-declarations -Werror

// 编译时将所有符号（包括动态链接库中的符号）添加到动态符号表中。
// 这样可以使用 dlopen() 和 dlsym() 动态地加载和调用符号。
-rdynamic

// 生成位置无关代码:将目标文件中的代码和数据设计为相对于程序运行时的位置的偏移量
-fPIC

// 添加搜索库文件的目录
-Ldir

// 添加链接的库
-llibrary

// 程序执行时，动态库的路径
// 方法一：环境变量 LD_LIBRARY_PATH
// 方法二：链接时指定
-Wl,-rpath[=/usr/local/lib],-rpath=[path]

// 自动生成依赖性,保存到文件filename,不包含系统头文件
-MMD -MF filename
```

## Makefile 模版
```
project
  --- src
     --- module
	 	--- module.c 	 
     --- main.c
  --- include
  --- bin 
  --- test 
  --- conf
  Makefile
```

```make
# 第一层 Makefile
# 定义各种变量
export BASEDIR := $(PwD)
export TARGET := main
export CFLAGS += -I$(BASEDIR)/include
export LDFLAGS

CFLAGS += -Wall -Werror -Wno-deprecated-declarations -O2

LDFLAGS += -L/usr/lib
LDFLAGS += -lpthread
LDFLAGS += -Wl,-rpath=/usr/lib

.PHONY:clean all

all:
	$(MAKE) -C $(BASEDIR)/src/ all

clean:
	$(MAKE) -C $(BASEDIR)/src/ clean
```

```make
# src 下的 makefile
TMP_SRC := $(shell find . -name "*.c")
SRC := $(subst ./, $(BASEDIR)/src/, $(TMP_SRC))
OBJS := $(patsubst %.c,%.o,$(SRC))

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)

.PHONY: clean all

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) -o $(BASEDIR)/src/$@ $^

%.o:%.c 
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MF $*.d -MP

clean:
	$(RM) $(TARGET) $(OBJS)

```


## Makefile的工作步骤

1. 读入所有Makefile
2. 读入被include的其他Makefile
3. 初始化文件中的变量
4. 推导隐含规则，并分析所有规则
5. 为所有目标创建依赖关系链：自顶向下递归创建
6. 根据依赖关系，决定哪些目标要重新生成
7. 执行生成命令


## Makefile书写规则
### 包含其他 Makefile 
```make
include filename
```

### 变量
变量赋值：
+ :=（简单赋值，立即展开）：用于进行简单的变量赋值操作。它会立即展开右侧的表达式，并将结果赋给左侧的变量。这意味着变量在定义时就会被展开，并且后续对该变量的引用将使用最初赋给它的值;

+ ?=（条件赋值，延迟展开）：如果变量尚未定义或为空，则会将右侧的值赋给左侧的变量。如果变量已经定义且非空，则保持其原有值，不进行重新赋值;

+ =（延迟展开赋值）：在变量使用的时候才会进行展开，而不是在定义时;

+ +=（追加赋值，延迟展开）：用于将右侧的值追加到左侧的变量上；


```make
VAR := hello
THEE := $(VAR)
TAG += $(VAR)
VAR := nihao
TAG += $(VAR)
VAR := buhao

clean:
	echo "~~~ $(THEE)"
	echo "~~~ $(TAG)"
```

预定义变量：
```make
CC: c编译器名称，默认cc
CPP : c预编译器名称，默认值$(CC) -E
CXX : C++编译器名称，默认g++
RM  : 默认 rm -f
CFLAGS:  c编译器选项
CXXFLAGS: c++编译器选项
LDFLAGS: 链接选项
```

自动化变量：
```make 
$@： 当前规则的目标文件；
$<： 当前规则的第一个依赖文件；
$^： 当前规则的所有不重复的依赖文件，以空格分隔；
$+:  所有依赖文件，以空格分隔，可能重复；
$*:  不包含扩展名的目标文件名称；
$?:  所有时间戳比目标文件晚的依赖文件，以空格分隔；
```


变量值替换:
```make
# 将foo中以.o结尾的字串换成以.c结尾的字串
foo := a.o b.o c.o
FOO := $(foo:.o=.c)

# 模式替换
POO := $(foo:%.o=%.c)
```
  
环境变量：
默认情况下，只有通过命令行设置的变量会被传递。而定义在文件中的变量，如果要向下层 Makefile 传递，则需要使用 `export` 关键字来声明。


目标变量：
作用范围只在规则及连带规则内。  
```
prog: CFLAGS = -g
prog: a.o b.o
    gcc $(CFLAGS) -c $@ a.o b.o

a.o: a.c
    gcc $(CFLAGS) -c $@ a.c
```

### 基本规则
```make
objects := main.o func.o 

$(TARGET):$(objects)
	$(CC) $(LDFLAGS) $(objects) -o $(TARGET)

main.o:main.c def.h 
	$(CC) -c main.c
func.o:func.c func.h 
	$(CC) -c func.c 

```

### 静态模式规则
```make 
objects = foo.o bar.o

$(TARGET):$(objects)
	$(CC) $(LDFLAGS) $(objects) -o $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
```
等价于
```make
foo.o : foo.c
	$(CC) -c $(CFLAGS) foo.c -o foo.o

bar.o : bar.c
	$(CC) -c $(CFLAGS) bar.c -o bar.o
```

### 自动生成依赖性
```make 
# 当头文件变化时，自动根据依赖性编译c文件
objects := foo.o bar.o

DEPS := $(objects:.o=.d)
-include $(DEPS)

$(TARGET):$(objects)
	$(CC) $(LDFLAGS) $(objects) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

```


### 语法
#### 条件选择
```make
ifeq ($(CC), gcc)
    objects = $(x)
else
    objects = $(y)
endif

### 判断是否为空
ifeq ($(ISX86), )
	objects += XXX
endif

ifdef DEBUG
	foo = yes
else
	foo = no
endif
```
 

#### 函数调用
`$(function variable1,variable2,...)`这是函数调用的基本形式。函数名与变量之间用空格分开，变量之间用`,`分开。注意不要有多余的空格

#### subst替换函数
```
comma = ,
empty = 
space = $(empty) $(empty)
foo := a.o b.o c.o
var := $(subst $(comma),$(space),$(foo))
echo $(var)
```

#### wildcard / patsubst 替换
```
# 列出文件夹中的所有 .c 文件
objects := $(wildcard *.c)


objects := $(patsubst %.c,%.o,$(wildcard *.c))
foo : $(objects)
	cc -o foo $(objects)
```

#### strip
去掉str开头和结尾的空格

#### findstring
`$(findstring a,a b c)`
查找字符串，如是找到，返回a, 找不到返回空


#### filter
```
foo := a.c b.c c.s d.h
$(filter %.c %.s,$(foo))
```
用模式去过滤字符串，（可以有多个模式）， 返回符合模式的字符串。上面就返回`a.c b.c c.s`

#### filter-out
返过滤函数，返回去除pattern之后的字符串

#### sort
`$(sort a c d d b e)`  
按升序排，会去除重复。 a b c d e

#### word
`$(word 2,bar foo get)`  
取第2个单词

#### wordlist
`$(wordlist 2,3,bar foo get word)`  
取单词串，`foo get`

#### word
`$(word bar foo get word)` 统计单词个数为4

#### dir/notdir
`$(dir src/foo.c hacks)` 结果是`src/ ./`  
`$(notdir src/foo.c hacks)` 结果是`foo.c hacks`

#### basename/suffix
取前缀、后缀函数  
`$(basename src/foo.c hacks)` 结果是：src/foo hacks  
`$(suffix src/foo.c hacks)`   结果是：.c

#### addprefix/addsuffix
`$(addprefix src/,foo bar)` 结果：src/foo src/bar  
`$(addsuffix .c,foo bar)`   结果：foo.c bar.c

#### join连接函数
`$(join aaa bbb,111 222 333)` 把后面的单词相应地加入到前面的单词后面。结果是：aaa111 bbb222 333

#### foreach
`$(foreach n,$(list),$(text))` 把list中的每个字串取出赋给n, 用n去执行text表达式，结果是一个字符串。每次返回的字符串，组成一个新的字符串作为最后的结果。  
```
list := a b c
$(foreach n,$(list),$(n).o)
```
结果是：a.o b.o c.o

#### if
`$(if <condition>, <then>, <else then>)`

#### call
`$(call <exp>,<parm1>,<parm2>,<parm3>...)`  
将后面的各个参数传入exp中，exp的返回值就是call函数的返回值  
```
reverse := $(2) $(1)
$(call reverse,a,b)
```
结果：b a

#### origin
`$(origin var)` 告诉你变量是从哪来的。  

+ undefined：未定义
+ default:  
+ environment: 环境变量（并且-e参数没有找开）
+ file：     定义在makefile中
+ command line：定义在命令行中
+ override：    被override重新定义了
+ automatic：   自动化变量

```
bletch := foo
ifdef bletch
ifeq ($(origin bletch), file)
    bletch := FOO
endif
endif
```
结果：FOO

#### shell函数
就是在Makefile中执行shell函数  
`content := $(shell pwd)`


### 书写命令
#### 显示命令
调试Makefile时，想要它输出命令但不执行   
```make
make -n
``` 

用来跟踪Makefile执行情况: @表示不显示命令，只输出qwert   
```make
@echo qwert
```

#### 命令执行
如果要让下一条命令在上一条的结果基础上执行，那这两条命令要放在一行  
```make
cd ~/Downloads; pwd
```

忽略命令的出错，我们可以在 Makefile 的命令行前加一个减号 - 
```make
clean:
	-rm -f *.o
```

嵌套执行 Makefile
```make
$(MAKE) -C subdir
```



### 在哪里寻找文件的依赖文件
Makefile永远首先在当前目录找依赖文件，找不到才去其他地方。  

在一个工程中，有大量的源文件，不同的源文件可能要在不同目录下去找依赖关系。Makefile有个特殊变量VPATH：  
`VPATH = SRC:../headers`  
上面指定了2个目录，src, ../headers， Makefile会按这个顺序去找  

还有一个更细的办法：  
`vpath <pattern> <directories>`  
`vpath %.h ../headers`  
%.h 要在../headers中去找

`vpath <pattern>`  
清除pattern模式的搜寻目录

`vpath`  
清除所有搜寻目录

如果同一pattern有多个搜寻目录，那就按先后顺序来

## 隐含规则 builtin-rule
Make 有一个隐含规则库，如果我们没有写出生成目标的命令，它会去规则库中按顺序找合适的命令。有时候会有一些错误，比如对同一目标有多条隐含命令，但你想执行的命令更靠后。

如果你不想执行隐含命令，就要显式写出来。

我的建议：尽量不要使用隐式规则。














