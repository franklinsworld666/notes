---
title: python 入门
tags: python
notebook: 1101 程序设计
---
[toc]

## 变量

### 变量类型

| 类型        | 举例                             |
| ----------- | -------------------------------- |
| int         |                                  |
| float       |                                  |
| bool        |                                  |
| complex复数 |                                  |
| str         |                                  |
| list        |                                  |
| tuple       |                                  |
| dict        |                                  |
| None        | 表示一个空对象，可赋值给任何变量 |

python 是如何知道变量类型的呢？它是在解释的时候，根据变量所引用的对象来自动推导数据类型的。

type(name) ： 获得 name 的类型
int(x): 把 x 转化为 int 类型
float(x): 把 x 转化为 float 类型
list(tuple): 把 tuple 转化成 list 类型
tuple(list): 转化成 tuple

### 全局变量和局部变量

python 中，一般无法在函数内修改全局变量

函数内部修改全局变量：

```python
global num
num = 10
```

### 变量的引用

```python
# a 引用了 1, id(1) == id(a) == 10914368
a = 1
# b 也引用了 1
b = a
# a 引用了 2, id(2) == 10914400
a = 2
```

对象就像是小盒子，变量就是一张标签，所谓引用就是把标签贴在小盒子上。
变量记录了对象的地址。

实参是通过引用传递的
函数的返回值也是通过引用传递的

### 可变类型与不可变类型

+ 不可变类型：数字，str，tuple
+ 可变类型：list，dict

函数内部，对于不可变类型对象的修改，会产生新的对象；参数是不可变类型，传值
函数内部，对于可变类型对象的修改，不会产生新的对象; 参数是可变类型，传引用

Python使用引用计数来管理内存，每个对象都有一个与之关联的引用计数，表示当前有多少个引用指向该对象。

+ 不可变对象:

  对于不可变对象（如整数、浮点数、字符串、元组），当你将一个变量赋值给另一个变量时，你只是在增加一个引用计数，并没有创建一个新的对象。这意味着两个变量将指向同一个对象。

  ```python3
  a =5
  b = a  # b现在引用了a指向的同一个整数对象
  ```
+ 可变对象（Mutable Objects）:

  对于可变对象（如列表、字典、集合），情况稍有不同。当你将一个变量赋值给另一个变量时，你同样只是在增加一个引用计数，两个变量指向同一个可变对象。

  ```python3
  my_list =[1,2,3]
  another_list = my_list  # another_list现在引用了my_list指向的同一个列表对象
  ```
+ 创造一个新的对象副本

  如果你想要创建一个新的对象副本，你需要显式地进行复制。对于不可变对象，这通常意味着创建一个新的对象，而对于可变对象，你可以通过切片操作、`copy()` 方法或 `deepcopy()` 方法来创建浅拷贝或深拷贝。

  ```python3
  my_list =[1,2,3]
  sub_list = my_list[:]# 创建一个浅拷贝

  import copy
  my_list =[[1,2],[3,4]]
  sub_list = copy.deepcopy(my_list)# 创建一个深拷贝
  ```

总结来说，对变量赋值的操作在Python中通常只是增加了一个引用计数，除非显式地进行复制操作。对于不可变对象，这通常不是问题，但对于可变对象，这可能导致意外的副作用，除非你理解并控制了引用的行为。


## 输入输出

```python
# input 一个字符串, input 获取到的都是字符串
passwd = input("Please input a passwd:")

```

```python
# 格式化输出
# %.2f 2位小数；%06d 6位整数不足用0补齐; %% 输出 %
print("格式化字符串 %s, %06d, %.2f, %.2f%%" % (var1, var2, var3, var4))

# print 打印完后，不换行
print("print not output a newline", end="")
```

### 文件

```python
# r, w, a, w+ etc
# w: 只写方式，不存在会创建
f = open(filename, "w+")
text = f.read()
f.write(text)
f.close()
```

```python
# 针对大文件读写
# readline 一次读一行
file = open("Readme", "a+")

while True:
    text = file.readline()
    if not text:
        break
file.close()
```

### 文件目录管理操作

```python
# 文件目录操作
import os

os.rename("old.txt", "new.txt")
os.remove("old.txt")
os.listdir("./")
os.isdir("./")
os.chdir("/")

```

## 运算

### 算术运算

| 运算    | 举例           |
| ------- | -------------- |
| + - *   |                |
| /       | 10 / 3 = 3.333 |
| 取整 // | 10 // 3 = 3    |
| 取余 %  | 10 % 3 = 1     |
| 幂 **   | 2 ** 3 = 8     |

### 逻辑运算

and,or, not

### 字符串操作

```python
"hello" + ",world"
"hello" * 5

he_str = "hello,world"

# 取值和取索引
he_str[0]
he_str.index("lo")

# 比较
"aaa" < "bbb"

# 查找
he_str.startswith("he")
# 成功，返回索引；失败，返回 -1
he_str.find("wor")

# 分割和连接字符串
# split 默认用空白符分割字符串，也可以指定
he_list = he_str.split()
" ".join(he_list)

# 统计
len(he_str)
he_str.count("ll")

# 去除空白字符
he_str.strip()

# 切片
# 切片范围 [start, end)
he_str[start:end:step]
```

### List 操作

List 采用动态数组来实现；
应用场景：存储多个相同类型的数据，然后遍历处理;
切片操作，是一个浅拷贝；

```python
# 取值和取索引
name_list = ["zhangsan", "lisi", "wangwu"]
name_list[1]
# 返回索引 1
name_list.index("lisi")

# 增加
# append add to the end
name_list.append("zhuliu")
name_list.insert(1, "xiaomei")
name_list.extend(another_list)
#### 注意，在 list 中 += 相当于 extend 方法，会修改 list  
name_list += name_list  

# 修改
name_list[1] = "lisi2"

# 删除
name_list.remove("lisi")
# pop remove the last item by default
name_list.pop()
name_list.pop(1)
name_list.clear()
# 不推荐用del 会从内存删除变量，再次访问会报错
del(name_list[1])

# 统计
# len 函数: list 中 item 个数
# count 方法：lisi 出现的次数
list_len = len(name_list)
num = name_list.count("lisi")

# 排序
# 升序排序，降序排序，逆序
name_list.sort()
name_list.sort(reverse=True)
name_list.reverse()


```

### deque

采用双链表来实现，对于头尾的插入、删除效率高；

```python3
# 双端链表
from collections import deque

d = deque()
d.append(x)

# 删除最后一个
d.pop()
# 删除第一个
d.popleft()
d.clear()

# 修改
d[i] = x 

# 查看
d[i]
d[0],d[-1]
count(x)

```

### tuple 操作

Tuple 特点：里面的元素不能修改
应用场景：存储多个不同类型的数据, 例如：函数的参数、返回值、格式化字符串

```python
# 定义一个元组
info_tuple = ("zhangsan", 18, 1.75)
info_tuple = ()
info_tuple = (5,)

# 取值和取索引
info_tuple = ("zhangsan", 18, 1.75) 
info_tuple[1]
# 返回索引 0
info_tuple.index("zhangsan")

# 统计
len(info_tuple)
info_tuple.count("zhangsan")
```

### dict 操作

dict 是采用 哈希表 来实现的，并不是有序的；

dict 的键必须惟一，而且需要是不可变的，因此键取数字、字符串、元组。

len_map = collections.defaultdict(lambda:0)

```python
# 定义
xiaoming_dict = {"name":"xiaoming",
            "gender":True,
            "age":18}


# add 
xiaoming_dict["age"] = 27
xiaoming_dict["nickname"] = "xiaoxiaoming"

# delete
xiaoming_dict.pop("name")
xiaoming_dict.clear()


# 查
if name in xiaoming_dict:
  
# 取值
xiaoming_dict["name"]

# 统计
len(xiaoming_dict)

# 获取全部的 value
xiaoming_dict.values()

# 遍历 dict
# key 是 dict 的关键字  
for key in xiaoming_dict:
    print("key:%s" % key)
    print(xiaoming_dict[key])
```

## 顺序、分支和循环

```python
# 分支流程， if 语句
if age >= 0 and age < 120:
    XXXX
elif XXX:
    XXXX
else:
    XXXX
```

```python
# while 循环
i = 0
result = 0
while i < 100:
    if i % 2 == 0:
        result += i
    i += 1
print(result)
```

```python
# for 循环
for name in name_list:
    print(name)

# else 中的代码，只有当 for 循环执行结束才会执行
# 如果 for 循环中 break 了，就不会执行 else
for num in num_list:
    print(num)
else:
    print("execute code")
```

## 函数

```python
def sum_num(num1, num2):
    """return the sum of num1 and num2
    :param num1: num1
    :param num2: num2
    """
    return num1 + num2


# 缺省参数默认值
def print_info(name, gender=True):
    """ 打印学生的姓名，性别

    :param name: 姓名
    :param gender: 性别
    """
    gender_txt = "男生"
    if not gender:
        gender_txt = "女生"
    print("%s is %s" % (name, gender_txt))

# 如果有多个缺省参数，可以指定参数名
def print_info(name, title="", gender=True):
    return
print_info("xiaomei", gender=False)
name_list.sort(reverse=True)


# 参数的个数不确定，多值参数
# *args: 元组
# **kwargs: 字典
def demo(name, *args, **kwargs):
    return
demo(1)
demo(1,2,3,4)
demo(1,2,3,4,age="18",gender="male")

# 拆包：对于支持多值参数的函数，把一个元组、字典传递给它，为了简化传递方式
num_list = [2,3,4]
info_dic = {"age":"18", "gender":"male"}
# 不拆包的写法
demo(1,2,3,4,age="18",gender="male")
# 拆包的写法
demo(1, *num_list, **info_dict)
```

### 公共函数

| 函数      | 举例               | 适用                                          |
| --------- | ------------------ | --------------------------------------------- |
| len(item) |                    | str,list,tuple,dict                           |
| del(item) |                    | as above                                      |
| max(item) |                    | 对于 dict，比较 key                           |
| min(item) |                    | as above                                      |
| 切片      |                    | str,list,tuple                                |
| +         | [1,2]+[3,4]        | str,list,tuple                                |
| *         | ["hi"]*5           | as above                                      |
| in        | 3 in (1,2,3)       | str,list,tuple,dict                           |
| not in    |                    | as above                                      |
| is        | a is b； a is None | id(a) == id(b)；a == b a和b引用的对象的值相等 |
| is not    | a is not b         | id(a) != id(b)                                |
| enumerate |                    |                                               |
|           |                    |                                               |

## 异常

```python
try:
    # try to exe these codes
except error_type:
    # process
except (error_type2, error_type3):
    # process
except Exception as e:
    print(e)
else:
    # 没有异常才会执行到这里
finally:
    # 无论是否有异常，都会执行的代码

print("~~~~~~~~~~~~")
```

异常的传递：
当函数或方法出现异常时，会将异常抛出给函数或方法的调用方；如果传递到主程序，仍然没有异常处理，程序才会被终止。
利用异常的传递性，可以在主程序中增加异常捕获，而不用在每个函数中捕获。

主动抛出异常，什么场景会这么做呢？
可以根据程序特有的业务需求，在不满足某个条件时，主动抛出异常，由异常处理模块捕获并处理。

```python
# Exception class
ex = Exception("密码长度不够")

# raise except-class
raise ex
```

## 面向对象

### 类的设计

+ 类名：大驼峰命名法
+ 属性：这类事物具有什么特征
+ 方法：这类对象具有什么行为

### 封装：根据职责把属性和方法封装到一个抽象的类中

对象初始化的时候，先分配一个空间，再调用初始化方法 __init__；在对象销毁前，会自动调用 __del__ 方法

```python
# 新式类：以 object 为基类， python3 默认是新式类
# 旧式类：不推荐使用  
# 为了在 python2 和 python3 中都使用新式类，推荐明确继承自 object 类
class woman(object):
    """ 女人类

    """

    def __init__(self, name):
        self.name = name
        self.__age = 18

    def secret(self):
        print("%s is %d " % (self.name, self.__age))
```

### 继承：实现代码的重用

```python
class Animal(objectj):
    def eat(self):
        print("eat")
  
    def drink(self):
        print("drink")

# 子类拥有父类全部的属性和方法
class Dog(Animal):
    def bark(self):
        print("bark")

# 如果父类的方法不能满足子类的需要，子类可以完全重写这个方法
class xiaotianquan(Dog):
    def bark(self):
        print("bark as a gold")
xiaoDog = xiaotianquan()
# bark as a gold
xiaoDog.bark()

# 或者对父类方法进行扩展
class xiaotianquan(Dog):
    def bark(self):
        print("bark as a gold")
        # 调用子类 Dog 的方法
        super().bark()


# 在子类中，不能访问父类的 私有属性 和 私有方法， 与 C++一致
# 在子类中，可以访问父类的 公有属性 和 公有方法， 与 C++ 一致
class xiaotianquan(Dog):
    def demo(self):
        # 调用子类的 eat 方法
        self.eat()


# 多继承，同时继承多个父类
# 如果父类 A 和 B 中具有同名的方法，最好不要出现这种情况，避免使用多继承  
# 多继承方法搜索顺序：由内置属性 __mro__ 来决定
# 对下面的 C 类，搜索顺序：C --- A --- B 
class C(A, B):

```

### 多态：不同的对象调用同样的方法，产生不同的结果

```python
class Dog(object):
    def game(self):
        print("normal dog play game")

class Xiaotianquan(Dog):
    def game(self):
        print("xiaotianquan flies to the sky to play")

def play_with_dog(dog):
        dog.game()

wangcai = Dog()
play_with_dog(wangcai)  # normal dog play game

wangcai = Xiaotianquan()
play_with_dog(wangcai)   # xiaotianquan filies to the sky to play
```

### 类的属性

类是一个特殊的对象

class AAA  定义的类属于类对象，类对象在内存中只有一份
aa = AAA()  属于实例对象

对象的内存空间中：

+ 类属性  :  类名.属性， 一般记录类的特征
+ 类方法
+ 实例属性
+ 实例方法

```python
class Tool(object):

    # 使用赋值语句，定义类属性，类似 C++ 的 static 成员
    count = 0

    def __init__(self, name):
        self.name = name
        Tool.count += 1

tool1 = Tool("futou")
# 访问类属性的方法
print(Tool.count)

```

属性获取机制

+ 首先，在实例对象内部查找
+ 如果没有，会向上查找，类中是否有该属性

### 类的方法和静态方法

实例方法：需要访问实例属性
类方法：不用访问实例属性，只访问类属性
静态方法：不访问实例属性，不访问类属性，不调用类方法，采用静态方法

```python
@classmethod
def fangfa(cls):
    print("this is a class method")

# how to use
类名.fangfa()

# 在类方法内部，访问类属性，或其他类方法
cls.类属性
cls.其他类方法


@staticmethod
def static_method():
    print("This is a static method")

# 类名.静态方法(), 调用静态方法
Dog.static_method()
```

## 模块

### 使用模块提供的方法

```python
import Module1
# 如果模块名太长，可起别名
import Module1 as M1
# 从模块中导入需要的
from Module2 import Cat
from Module2 import say_hello

Module1.func()
Module1.Dog()

ccc = Cat()
say_hello()

# 如果从不同模块导入相同名称函数，后者会覆盖前者
# 解决方法：给另一个函数起一个别名
```

### 模块的搜索顺序

+ 首先搜索当前目录，寻找指定文件名的 py 文件，如果有就加载
+ 找不到，才搜索系统目录

```python
# __file__ 显示模块的完整路径
print(random.__file__)
```

### 包

包里面包含了多个模块，以及一个 `__init__.py` 文件，这个文件中包含了允许外界使用的模块

```python
# package: hm_msg
# __init__.py's content
from . import send_msg
from . import recv_msg

# 使用包
import hm_msg
hm_msg.send_msg()
hm_msg.recv_msg()
```

### random 随机数

```python
import random
# [12, 20] random num
random.randint(12, 20)
```

## 其他

指定文本编码格式

```python
# *-* coding:utf-8 *-*

```

多行注释

```python
"""
这是一个多行注释，三个双引号
"""
```

查看对象的方法: 按 tab 键就可以显示
使用 `dir(对象)` 可以查看对象的所有属性及方法

函数注释

```python
def print_info(name, gender):
    """ 打印学生的姓名，性别

    :param name: 姓名
    :param gender: 性别
    """
```

python 运行分两步：先将源码转化为字节码，再将字节码转化为机器码。pyc 文件就是预先转化好的字节码。

pip3 是 python3 包管理工具

```python
sudo pip3 install pygame
sudo pip3 uninstall pygame
```

### 单例设计模式

应用场景：系统中只有一个对象的场景，音乐播放对象，回收站对象，打印机对象等

特点：每次返回的对象都是同一个内存地址的

```python
class MusicPlayer(object):

    # 类属性初值 None
    instance = None
    init_flag = False

    def __init__(self):
        if MusicPlayer.init_flag:
            return
        MusicPlayer.init_flag = True
        print("MusicPlayer")

    def __new__(cls, *args, **kwargs):
        if cls.instance is None:
            cls.instance = super().__new__(cls)
        return cls.instance

player = MusicPlayer()
print(player)
```
