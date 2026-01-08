---
title: mysql 入门
tags: 数据库
notebook: 1101 程序设计
---

[TOC]

## 数据库操作
```mysql
# 创建，默认编码是 latin1 --- 一种向下兼容ASCII的单字节编码
create database Yao;

# 先检查是否存在，不存在就创建
create database if not exists Yao;

# 查看某个数据库是怎么创建的
show create database Yao;

# 查看所有数据库
show databases;

# 显示当前所在的数据库
select database();

# 使用数据库
use Yao;

# 丢弃数据库
drop database Yao;
```

## 数据类型
```mysql
INT 4
DECIMAL(m,d)  定点型浮点数，m 代表总个数 d小数位, m最大65
CHAR 0-255字节的定长字符串   CHAR(4)  存储“2", 实际"0002"
VARCHAR 0-65535的不定长字符串 VARCHAR(4) 存储"2" 实际"2"
TEXT   0-65535    长文本数据
datetime   4   yyyy-mm-dd hh:mm:ss
TINYINT  1
SMALLINT 2
MEDIUMINT 3
BIGINT 8
```

## 创建和删除表
```mysql
# 显示当前数据库所有的表
show tables;

# 显示表结构
desc Yao.student;
desc stduent;

# 显示表是怎么创建的
# \G: 显示更易读
show create table student\G;

# 创建表
create table student (
    id INT auto_increment unique key comment 'stu no',
    name VARCHAR(24) not null,
    sex CHAR(1) not null,
    class CHAR(4) default '0304'
    );

# 复制另一张表的表结构
create table teacher like student;

# 复制另一张表的结构及数据
create table teacher as select * from student;
create table teacher as select Grade,Class from student;

# 删除表 
drop table student;
```

## 插入表 
```mysql
# 普通插入
# insert into 表名（字段名） values（字段对应值）;
insert into employee (empno,ename,job,mgr,hiredate,sal,deptnu) values ('1000','小明','经理','10001','2019-03-03','12345.23','10');

# 普通插入
# insert into 表名 values（所有字段对应值）;
insert into employee  values ('1001','小明','经理','10001','2019-03-03','12345.23','10');  

# 一次插入多个数据
insert into 表名  (字段名) values (对应值1),(对应值2),(对应值3);


# 将一张表的数据复制到另一章
# insert into 表名1 select * from 表名2;
insert into emp select * from employee;
​
# 将一张表的数据复制到另一章
# insert into 表名1（字段名1，字段名2） select 字段名1，字段名2 from 表名2;
insert into emp (empno,ename) select empno,ename from employee;

# 建表时复制
```

## 查询表
```mysql
# 条件查询
select * from employee where sal != 50000;
select * from employee where ename='后裔';
select * from employee where sal between 10000 and 30000;
select * from employee where hiredate between '2011-01-01' and '2017-12-1';
select * from employee where ename in ('猴子','林俊杰','小红','小胡');  
select * from employee  where ename like '林%';

# 普通查询 --- 尽量不要，太慢了
select * from employee;
select empno,ename,job as ename_job from employee;

# 统计查询
select distinct(job) from employee;
select count(ename) from employee;
select sum(sal) from employee;
select * from employee where sal= (select  max(sal) from employee);
select avg(sal) from employee;
select * from employee where sal= (select  min(sal) from employee);
# concat 起连接作用
select concat(ename,' 是 ',job) as aaaa from employee;

# 关键字在语句中的顺序：where, group, having, order by， limit
# 关键字执行顺序：from on Where, Group By, Having, select，distinct，order by，limit
# 聚合函数（count sum min max等）是针对结果集进行的，但是where条件并不是在查询出结果集之后运行，所以聚合函数放在where语句中，会出现错误

# group by
select deptnu,count(*) from employee group by deptnu;
select deptnu,job,count(*) from employee group by deptnu,job;

# having
select job,count(*) from employee group by job having job ='文员';
select  deptnu,job,count(*) from employee group by deptnu,job having count(*)>=2;

# order By
select * from employee order by hiredate;
select  deptnu,job,count(*) as 总数 from employee group by deptnu,job having 总数>=2 order by deptnu desc;
select  deptnu,job,count(*) as 总数 from employee group by deptnu,job having 总数>=2 order by deptnu;

# limit
# limit n,m: n 代表起始，默认0；m 代表条数
select * from XD.employee limit 4,5;

# exist 查询
# exists型子查询后面是一个受限的select查询语句
# exists子查询，如果exists后的内层查询能查出数据，则返回 TRUE；为空则返回 FLASE
select * from 表名 as a where exists (select 1 from 表名2 where 条件);
select * from dept a where exists (select 1 from employee b where a.deptnu=b.deptnu);
```

### 其他函数
|符号|作用|
|---|---|
|:=| select update时都起到赋值作用|
|IS NULL| NULL代表未定义的值，NULL与NULL不能比较 |
|IS NOT NULL||
|if(exp1,exp2,exp3)|exp1 is true,then exp2,else exp3|
|IFNULL（x,y）|若x是NULL，返回y。这里x y是表达式|
|_  |下划线代表一个字符|
|%  |多个字符|
|NOT IN||
|NOT EXIST|NOT EXIST (SELECT ...) select没有数据，返回TRUE|
|ALL|WHERE price > ALL (SELECT ...),大于集合的全部|
|ANY，SOME|大于集合的任一个|
|LENGTH(string)|长度|
|UPPER(string) LOWER(string)||
|REVERSE(string)||
|RIGHT(columnName,2)|列中从右开始2个字符|
|LEFT(columnName,2)||
|SUBSTRING_INDEX(columnName,',',1)|第一个,前的数据|
|SUBSTRING(string,startPos, length)|字符串中从开始位置的length长度，起始位置从1算|
|LTRIM(string)|清楚左侧多余空格后的字符串|
|RTRIM(string)||
|ORDER BY sales DESC|DESC means descent|
|LIMIT x,y/limit y offset x|从x开始，显示y条记录，0是第一条|
|PI(),RAND()||
|POWER(X,Y) ROUND(X,Y)|x的y次方，x四舍五入保留y位小数|
|SIN(X) SQRT(X) TAN(X)||
|ABS(X)|绝对值x|
|TRUNCATE(X,Y)|X截断，保留y位小数|
|CAST(2/2.00 AS DECIMAL/SIGNED）|2.00/2|
|DATEDIFF(exp1,exp2)|exp1-exp2|日期计算|
|DATA_FORMAT(a_data,'%M %Y')|处理时间格式字符串|

## 联接和子查询
|类型|作用|
|---|---|
|left join/left outer join|匹配左表每一行及右表中符合条件的行，没有匹配就显示NULL|
|right join/right outer join|匹配右表每一行及左表中符合条件的行，没有匹配就显示NULL|
|inner join/join/cross join/,|第一张表和第二张表做一个笛卡尔积，但几种写法之间还是有细微差别；如：on必须跟在join后面；on条件如果不匹配，不显示|
|union|select 包含的字段数要一致，字段数据类型要相近；会显示重复数据|
|union all|区别是：不显示重|
|INTERSECT|交集|
|EXCEPT|差集|
|子查询劣势|SELECT子查询都可以用联接替换，联接更好用|
|子查询优势|可以和INSERT DELETE UPDATE联用|
|where型子查询|把内层查询结果作为外层查询的比较条件，子查询返回一个值（除了In返回单列）|
|from型子查询|内层sql的查询结果，当成临时表，供外层sql再次查询|
|exists型子查询|把外层查询结果拿到内层sql测试，如果成立，则取出|
|NATURAL JOIN|内联接的特殊情况，自动识别每个表的相同名称并返回相符记录|

```MYSQL
# 外连接：左连接，右连接
# 列出部门名称和这些部门的员工信息，同时列出那些没有的员工的部门 
select a.dname,b.* from dept a left join employee b on a.deptnu=b.deptnu;
select b.dname,a.* from employee a right join dept b on b.deptnu=a.deptnu;

# 内连接
# 想查出员工张飞的所在部门的地址
select a.addr  from dept a inner join employee b on a.deptnu=b.deptnu and b.ename='张飞';

# 联合查询：把多个查询语句的结果结合在一起
# 对销售员的工资从低到高排序，而文员的工资从高到低排序
(select * from employee a where a.job = '销售员'  order by a.sal limit 999999 ) union  (select * from employee b where b.job = '文员' order by b.sal desc limit 999999);

# 子查询 = 外层查询 + 内层查询
# 1. where
SELECT firstName FROM my_contacts
WHERE zipCode = (SELECT zipCode FROM Address WHERE state = 'TN');

SELECT firstName FROM my_contacts
WHERE type IN (SELECT type FROM Address WHERE state = 'TN');

# 2. from型
select * from
(select state from Address) as temp;

# 3. exists型
select mc.firstName from my_contacts mc
where exists (select state from Address
where zipCode = mc.zipCode);

# natural join 
SELECT mc.firstName FROM my_contacts mc
NATURAL JOIN job_current jc
WHERE jc.salary > (SELECT AVG(salary) FROM job_current);

``` 

## 修改和删除表数据
```mysql
# 修改某几列的值
update 表名 set 字段名1=值1,字段名2=值2 where 字段名=值;

# 修改多行的值 
update myContacts 
set firstName =  
CASE 
    WHEN contactId = 2 THEN 'Donald'
    WHEN contactId = 3 THEN 'Jack'
    WHEN contactId = 4 THEN 'Ivank'
    ELSE 'misc'
END;

# 删除某列
delete from student where id=1;

# 删除整张表的数据，但表的定义还在，同时会记录操作方便回滚
delete from student;

# 删除整张表的数据，但表定义还在，不会记录操作无法回滚，占用的空间也会被释放
truncate from student;

# 删除整张表，包括表的定义
drop table student;
```

## 修改表结构
```mysql
# 修改表名
rename table student to user;

# 在表中增加一列
# alter table student add 列名 类型 (约束条件) (位置) 
alter table student add addr VARCHAR(16) not null default 'JY' after sex;

# 修改列类型 
alter table student modify addr VARCHAR(32);

# 修改列的名称 
alter table student change addr address VARCHAR(16);

# 给列加索引 
alter table student add index(name);

# 删除列
alter table stduent drop addr;

# 修改表的字符集
alter table student character set gbk;
```

## 索引
+ MyISAM与InnoDB的区别
```
MyISAM：支持全文索引（full text）;不支持事务;表级锁;保存表的具体行数;奔溃恢复不好
​
Innodb：支持事务;在5.6之后的版本就开始支持全文索引;行级锁（并非绝对，当执行sql语句时不能确定范围时，也会进行锁全表例如： update table set id=3 where name like 'a%';）;不保存表的具体行数;奔溃恢复好
```

+ 如何选择引擎
```
推荐：用InnoDB

InnoDB：
1. 可靠性要求高的，或者要求支持事务
2. 想要用到外键约束的时候（讲外键的时候会讲）

MyISAM：
1. 一般来说MyISAM不需要用到事务的时候
2. 做很多count计算
```

+ 索引的优点、缺点
```
优点：
通过创建唯一索引，来保证数据库表中的每一行数据的唯一性。
可以加快数据的检索速度

缺点：
索引需要占用物理空间。
对表中的数据进行改动时，索引也需要跟着动态维护，降低了数据的维护速度。
```

+ 索引的常见类型
```
index：普通索引,最为普通的索引，主要任务就是提高查询速度。其特点是允许出现相同的索引内容，允许空（null）值

unique：唯一索引,不可以出现相同的索引内容，但是可以为空（null）值

primary key：主键索引,它是一种特殊的唯一索引，不允许有空值.

foreign key：外键索引

fulltext: 全文索引, 是将存储在数据库中的文章或者句子等任意内容信息查找出来的索引，单位是词。

组合索引
```

+ 生成百万级数据表
```
#!/bin/bash
​
echo "请输入字段servnumber的值："
read serber
echo "请输入创建sql语句的数量："
read number
​
# char=`head /dev/urandom | tr -dc 0-9 | head -c 11`
​
for (( i=0;i<$number;i++ ))
        do
        pass=`head /dev/urandom | tr -dc a-z | head -c 8`
        let serber=serber+1
        echo "insert into test(id,username,servnumber,password,createtime) values('$i','user${i}','${serber}','$pass',now());" >>sql.txt
​
    done
---------------------
sh test.sh
in mysql cmd: source sql.txt
```

+ 创建、删除索引
```
# 建表时添加索引
create table test (
    id int(7) zerofill auto_increment not null,
    username varchar(20),
    servnumber varchar(30),
    password varchar(20),
    createtime datetime,
    /index/unique/fulltext (id)
)DEFAULT CHARSET=utf8;

# 修改添加索引
alter table 表名 add index (字段名称);
alter table test add unique (username);
alter table 表名 add primary key (字段名);
alter table 表名 add fulltext (字段名);

# 查看索引
show index from 表名\G;

# 删除索引
# 在有自增的情况下，必须先删除自增，才可以删除主键
alter table 表名 drop index 索引名;
alter table 表名 drop unique 索引名;
alter table 表名 drop primary key;
# 删除 fulltext
alter table 表名 drop index 索引名;

# 使用全文索引
# select * from 表名 where match  (字段名) against ('检索内容');
select * from command where match(instruction) against ('sections');

# 在全文索引查询时，采用boolean模式， 还存在其他复杂语法
# 全文索引针对的是词，如果词不写全，可以用此模式
select * from 表名 where match (字段名) against ('director*' in boolean mode);

# 全文索引的注意点
一般情况下创建全文索引的字段数据类型为 char、varchar、text 。其它字段类型不可以

全文索引不针对非常频繁的词做索引。比如is，no，not，you，me，yes这些，全文索引可能失效
​
对英文检索时忽略大小写
```

## 外键
外键约束，用于维护两个表间的数据一致性。比如，学生表中有一个班级号，班级号又是班级表的主键，这时可以把学生表的班级号设为外键，用以关联两个表。

+ 学生表（添加单一外键）

|sno(pk)|sname|classNo(fk)|
| --- | --- | --- |
|1  |   jack |       100|
|2  |   lucy |       100|
|3  |   king |       200|

+ 班级表

| cno(pk) |cname  |  |
| --- | --- | --- |
| 200 | 浙江省第一中学高三2班 |  |
| 100 | 浙江省第一中学高三1班 |  |

为了保证学生表中的classno字段中的数据必须来自于班级表中的cno字段中的数据，有必要给学生表中的classno字段添加外键约束

+ 注意点:
```
通过外键关联的两个列，数据类型必须相似
外键字段去引用的字段，必须unique，不然它到底引用哪一个呢
外键值可以为null
有了外键引用之后，表分为父表和子表，班级表：父表，学生表：子表
创建先创建父表，插入先插入父表数，据删除先删除子表数据
外键约束，必须是 InnoDB 引擎
```

+ 创建外键
```MYSQL
mysql> drop table if exists t_student;
mysql> drop table if exists t_class;

mysql> create table t_class(
    -> cno int(10) primary key,
    -> cname varchar(128) not null unique
    -> );

mysql> create table t_student(
    -> sno int(10) primary key auto_increment,
    -> sname varchar(32) not null,
    -> classno int(3),
    -> foreign key(classno) references t_class(cno)
    -> );
```

+ 添加外键 
```
alter table employee add foreign key (deptnu) references dept(deptnu);
```

+ 删除外键约束，外键索引
```
# 先要删除外键约束，才能删除外键索引
alter table employee drop foreign key employee_ibfk_1;
alter table employee drop index deptnu;
```

## 联合索引
+ 什么是索引
```
联合索引又称组合索引或者复合索引，是建立在俩列或者多列以上的索引。联合索引的效率远远高于单列索引。
```

+ 创建联合索引
```
# alter table 表名 add index(字段1,字段2,字段3);
​
alter table test add index(username,servnumber,password);
```

+ 删除联合索引
```
alter table test drop index username;
```

+ 联合索引的最左原则
```
不管有多少个字段，添加联合索引时最左边的字段最重要，查询的时候条件里必须要有这个字段，联合索引才会生效。
```

+ 联合索引的注意事项
```
索引并非越多越好，过多的索引会增加数据的维护速度还有磁盘空间的浪费。
​
当表的数据量很大的时候，可以考虑建立索引。
​
表中经常查数据的字段，可以考虑建立索引。
​
想要保证表中数据的唯一性，可以考虑建立唯一索引。
​
想要保证俩张表中的数据的完整性跟准确性，可以考虑建立外键约束。
​
经常对多列数据进行查询时，可以考虑建立联合索引。
```

## 优化
### 开启慢查询日志
+ 查看是否已经开启了慢查询日志
```
mysql> show variables like 'slow%';
+---------------------+--------------------------------------+
| Variable_name       | Value                                |
+---------------------+--------------------------------------+
| slow_launch_time    | 2                                    |
| slow_query_log      | OFF                                  |
| slow_query_log_file | /data/mydata/xdclass-public-slow.log |
```

+ 开启慢查询日志
```
set global slow_query_log = on ;

# 日志路径也可以自定义：
set global slow_query_log_file = '路径';
```

+ 查看慢查询的时间临界值
```
show variables like '%long%';
```

+ 设置慢查询的时间标准
```
set long_query_time=0.4;

# 重启mysql服务会让在交互界面设置的慢查询恢复到默认
# 永久生效的设置方法：修改配置文件 vi /etc/my.cnf
# 最后必须重启服务才能生效！
[mysqld]
slow_query_log = 1
long_query_time = 0.1
slow_query_log_file =/usr/local/mysql/mysql_slow.log

```

### 开启性能详情
+ 查看性能详情是否开启
```
mysql> show variables like '%profiling%';
+------------------------+-------+
| Variable_name          | Value |
+------------------------+-------+
| have_profiling         | YES   |
| profiling              | OFF   |
| profiling_history_size | 15    |
+------------------------+-------+
```

+ 开启性能记录功能
```
set profiling = on ;
```

+ 查看性能的记录
```
mysql> show profiles;
+----------+------------+---------------------------------------------------+
| Query_ID | Duration   | Query                                             |
+----------+------------+---------------------------------------------------+
|        1 | 0.00177775 | show variables like '%profiling%'                 |
|        2 | 0.00037900 | select * from test where id='087878'              |
|        3 | 0.34618025 | select * from test where servnumber='1367008787'  |
|        4 | 0.31986825 | select * from test where servnumber='13670087879' |
+----------+------------+---------------------------------------------------+
```

+ 查看语句的执行性能详情
```
mysql> show profile for query 4;
+----------------------+----------+
| Status               | Duration |
+----------------------+----------+
| starting             | 0.000100 |
| checking permissions | 0.000010 |
| Opening tables       | 0.000023 |
| init                 | 0.000045 |
| System lock          | 0.000015 |
| optimizing           | 0.000016 |
| statistics           | 0.000028 |
| preparing            | 0.000020 |
| executing            | 0.000006 |
| Sending data         | 0.319489 |
| end                  | 0.000037 |
| query end            | 0.000012 |
| closing tables       | 0.000012 |
| freeing items        | 0.000040 |
| cleaning up          | 0.000017 |
+----------------------+----------+
```

+ 性能线程的详细解释官方文档链接：
```
# General Thread States
https://dev.mysql.com/doc/refman/5.7/en/general-thread-states.html

# Sending data
The thread is reading and processing rows for a SELECT statement, and sending data to the client. Because operations occurring during this state tend to perform large amounts of disk access (reads), it is often the longest-running state over the lifetime of a given query.
```


### sql 语句注意点
```
尽量避免使用select *from ，尽量精确到想要的结果字段

尽量避免条件使用or

记得加上limit 限制行数，避免数据量过大消耗性能

使用索引查询时，条件字段类型转换会使索引失效，%放在前面会使索引失效
​
mysql> explain select * from test where servnumber like '%1367000%'\G
*************************** 1. row ***************************
           id: 1
  select_type: SIMPLE
        table: test
   partitions: NULL
         type: ALL
possible_keys: NULL
          key: NULL
      key_len: NULL
          ref: NULL
         rows: 996303
     filtered: 11.11
        Extra: Using where

```


## 事务
+ 什么是事务？
```
事务通常指对数据库进行读或写的一个操作过程。有两个目的，
第一个是为数据库操作提供了一个从失败中恢复到正常状态的方法，同时提供了数据库即使在异常状态下仍能保持一致性的方法；
第二个是当多个应用程序在并发访问数据库时，可以在这些应用程序之间提供一个隔离方法，以防止彼此的操作互相干扰。
```

+ 事务的特性（ACID）：
```
原子性(Atomicity)：一个事务中的所有语句，应该做到：要么全做，要么一个都不做；

一致性(Consistency):让数据保持逻辑上的“合理性”，比如：小明给小红打10000块钱，既要让小明的账户减少10000，又要让小红的账户上增加10000块钱；

隔离性(Isolation)：如果多个事务同时并发执行，但每个事务就像各自独立执行一样。

持久性(Durability)：一个事务执行成功，则对数据来说应该是一个明确的硬盘数据更改（而不仅仅是内存中的变化）。
```

+ 事务的用法
```
事务的开启：begin; start transaction;
事务的提交：commit;
事务的回滚：rollback;

# 默认 autocommit 是 on，所以即使没有开始事务，没有commit，也会自动commit
show variables like 'autocommit';
```

## 视图VIEW
+ 什么是视图？视图的作用是什么？
```mysql
View 是一种虚拟的逻辑表，它本身是不包含数据的。作为一个select语句保存在数据字典中的。
通过视图，可以只展现基表的部分数据。
```

+ 视图的优点是：
```mysql
使用视图的大部分情况是为了保障数据安全性，提高查询效率。

1）简单：对用户来说已经是过滤好的符合条件的结果集。
2）安全：使用视图的用户只能访问他们被允许查询的结果集，对表的权限管理并不能限制到某个行某个列，但是通过视图就可以简单的实现。
3）数据独立：一旦视图的结构确定了，可以屏蔽表结构变化对用户的影响，源表增加列对视图没有影响;源表修改列名，则可以通过修改视图来解决，不会造成对访问者的影响。
4）不占用空间：视图是逻辑上的表，不占用内存空间
```

+ 视图的缺点
```mysql
 1)性能差：sql server必须把视图查询转化成对基本表的查询，如果这个视图是由一个复杂的多表查询所定义，那么，即使是视图的一个简单查询，sql server也要把它变成一个复杂的结合体，需要花费一定的时间。
 
 2)修改限制：当用户试图修改试图的某些信息时，数据库必须把它转化为对基本表的某些信息的修改，对于简单的试图来说，这是很方便的，但是，对于比较复杂的试图，可能是不可修改的。
```

+ 视图的用法
```mysql
# 创建的基本语法是：
    create view <视图名称> as select 语句;
    create view <视图名称> (字段) as select 语句;
    create or replace view <视图名称>;

# 修改的语法是：
alter view <视图名称> as select 语句;

# 视图删除语法：
drop view <视图名称> ;

# 在查询中使用视图
SELECT * FROM pb;
```

## 其他语句
```mysql
# 创建时指定数据库的字符集
create database Yao default character set utf8;

# 查看某个库是什么字符集
show create database Yao;

# 查看当前 mysql 使用的字符集
show variables like 'character%';

# 创建临时表
CREATE TEMPORARY TABLE my_temp_table
(
    id INT,
    some_data VARCHAR(32)
);

```
## 其他
+ 查看 mysql 状态
```
service mysql status
```

+ 启动 mysql 
```
service mysql restart
```

+ mysql 登陆
```mysql
mysql -uroot -p
```

+ mysql 服务默认端口: 3306  

+ 关于 mysql 的编码问题
```mysql
# 使用的字符集
character_set_client：客户端请求数据的字符集
character_set_connection：客户端与服务器连接的字符集
character_set_database：数据库服务器中某个库使用的字符集设定，如果建库时没有指明，将默认使用配置上的字符集
character_set_results：返回给客户端的字符集(从数据库读取到的数据是什么编码的)
character_set_server：为服务器安装时指定的默认字符集设定。
character_set_system：系统字符集(修改不了的，就是utf8)
character_sets_dir：mysql字符集文件的保存路径

# 临时修改字符集，退出 mysql 失效
set names gbk;

# 永久修改字符集，要修改配置文件 my.cnf
# 作用于外部显示的
[client]
default-character-set=gbk
​
# 作用于内部，会作用于创建库表时默认字符集
[mysqld]
character_set_server=gbk

# 修改库的字符集
alter database xiaoxiao default character set gbk;

# 修改表的字符集
alter table employee default character set utf8;
```

+ mysql 安全相关
```mysql
# 查看root用户可以在哪台机器登录
select user,host from mysql.user where user='root';

# 修改mysql库里边的user表
update mysql.user set host='localhost' where user='root';

# 刷新权限
flush privileges;

# 修改 root 用户登陆密码
mysqladmin -uroot -p123456 password;

# 创建新用户，指定登陆主机ip，设置登录密码
create user 'luren'@'%' identified by '123456';

# 删除用户
# drop user 'username'@'host';
drop user 'luren'@'%';

# 授权语法
# grant 权限1,权限2..... on 数据库对象 to '用户'@'host' identified by 'password';
grant all privileges on *.*  to 'pig';
grant insert on XD.employee to 'cat'@'%' identified by '123456';
flush privileges;

# 回收语法
# revoke 权限1,权限2..... on 数据库对象 from '用户'@'host';
revoke all privileges on *.*  from 'pig' @ '%';
flush privileges;

```

+ mysql 备份
```
# 为什么要备份
为了数据安全，为了误操作之后恢复等

# 备份类型
完全备份：对整个数据库的数据进行备份
增量备份：是以上一次备份为基础来备份变更数据的，节约空间
差异备份：是以第一次完全备份的基础来备份变更备份的，浪费空间

# 备份方式
逻辑备份：直接生成sql语句保存起来，在恢复数据的时候执行备份的sql语句来实现数据的恢复
物理备份：直接拷贝相关的物理数据

# 远程备份单库例子并保留创建库语句：
mysqldump -uroot -pabc123456 -h120.25.93.69 --databases zabbix | gzip  > /mysql_data_back/zabbix_bak.sql.gz

# 原创备份多库的例子
mysqldump -uroot -pabc123456 -h120.25.93.69 --databases zabbix XD | gzip  > /mysql_data_back/zabbix_XD.sql.gz

# 恢复备份
gunzip zabbix_bak.sql.gz
mysql -uroot -pabc123456 -h120.25.93.69  < zabbix_bak.sql

# 物理备份方法一：拷贝数据源文件
# 一般不会这么做

# 找数据库源文件路径：
mysql> show variables like 'datadir%';
+---------------+---------------+
| Variable_name | Value         |
+---------------+---------------+
| datadir       | /data/mydata/ |
+---------------+---------------+
​
# MyISAM表源文件：
# 备份的时候，把 mysql 停了
db.opt：创建库的时候生成，主要存储着当前库的默认字符集和字符校验规则
.frm ：记录着表结构信息的文件
.MYI：记录着索引的文件
.MYD :记录着表的数据

# InnoDB表源文件：InnoDB有着共享表空间跟独立表空间的概念。
# 备份的时候，把 mysql 停了
db.opt：创建库的时候生成，主要存储着当前库的默认字符集和字符校验规则
.frm ：记录着表结构信息的文件
.ibd ：独立表空间，里边记录这个表的数据和索引
ibdata1：共享表空间，里边记录表的数据和索引
ib_logfile0
ib_logfile1

```

+ 二进制日志 mysqlbinlog
```
# 什么是二进制日志：
二进制日志就是记录着mysql数据库中的一些写入性操作，比如一些增删改，但是，不包括查询！

# 二进制日志有哪些功能：
一般情况下，二进制日志有着数据复制和数据恢复的功能; 开启后，会有1%的性能消耗

# 查看二进制日志是否开启：
how variables like 'log_bin%';

# 开启二进制日志 : vi /etc/my.cnf
[mysqld]
log-bin=/data/mydata/log_bin/mysql-bin
server-id=1

service mysql restart

# 查看所有的binlog日志列表
mysql> show master logs;
+------------------+-----------+
| Log_name         | File_size |
+------------------+-----------+
| mysql-bin.000001 |     23638 |
+------------------+-----------+

# 刷新二进制日志：会建立新的日志文件
flush logs;

# 重置日志文件
reset master;


```

+ 表的规范化
```
# 多对多的两张表，怎么变得规范呢？
通过增加一张连接表C，实现A-C B-C为一对多的关系。

# 数据原子性
列中的数据被解析出查询所需的最小单位
  
# 第一范式1NF
规则一：数据列只包含具有原子性的值
规则二：没有重复的数据组

# 第二范式
满足第一范式
没有部分函数依赖性
  
# 第三范式
满足第二范式
没有传递函数依赖
```

## 习题
+ 查出至少有一个员工的部门。显示部门编号、部门名称、部门位置、部门人数
```mysql
select a.deptnu,count(*) from dept a inner join employee b on a.deptnu = b.deptnu group by a.deptnu;

select a.deptnu, b.zongshu from dept a,(select deptnu,count(*) as zongshu from employee group by deptnu) b where a.deptnu=b.deptnu;
```

+ 列出薪金比安琪拉高的所有员工
```
select * from  employee where sal > (select sal from employee where ename='安琪拉');
```

+ 列出所有员工的姓名及其直接上级的姓名 
```mysql
select a.ename,ifnull(b.ename,'boss') as leader from employee a left join employee b on a.mgr=b.empno;
```

+ 列出所有文员的姓名及其部门名称，所在部门的总人数
```mysql
select b.ename,a.dname,b.job,c.zongshu from dept a ,employee b ,(select deptnu,count(*) as zongshu from employee group by deptnu) c where a.deptnu=b.deptnu and b.job='文员' and b.deptnu=c.deptnu;
```

+ 两列字段互换
```
update table1 a, table1 b
set a.field_1 = b.field_2, a.field_2 = b.field_1
where a.id = b.id
```

+ 交换上下两行
```
select s1.id,IFNULL(s2.student,s1.student) student from seat s1 
left join seat s2 
on if(s1.id mod 2 = 1,s1.id=s2.id-1,s1.id=s2.id+1) 
ORDER BY s1.id
```

+ 查询最小值所在行
```
select a.* from tb a where val = (select min(val) from tb where name = a.name) order by a.name
```

+ 查询每个分组前二名
```
select a.* from tb a where 2 > (select count(*) from tb where name = a.name and val > a.val ) order by a.name,a.val 

select a.* from tb a where exists (select count(*) from tb where name = a.name and val > a.val having Count(*) < 2) order by a.name 
```

+ 查询分数排名
```
SELECT t.Score Score, CAST(
(CASE WHEN @preScore = t.Score THEN @preRank
      WHEN @preScore := t.Score THEN @preRank := @preRank+1
      ELSE @preRank := @preRank+1
       END) AS SIGNED)Rank
FROM (SELECT Score FROM Scores ORDER BY Score DESC)t,(SELECT @preScore := NULL, @preRank := 0)q;
```

