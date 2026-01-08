---
title: markdown
tags: 
notebook: 
---

[TOC]

## 全文结构
文章的结构是通过多级标题来划分的。

```
# 一级标题
## 二级标题
### 三级标题

+ 无序列表1
+ 无序列表2
	+ 无序列表3

1. 有序列表一
2. 有序列表二
	1. 有序列表1
```



## 强调文字  

*这是斜体*  

**对文字加粗** 

***这是斜粗体***

`这是高亮文字`

<u>这是下划线</u>

~~两个波浪号是删除线~~

换行一：末尾2个空格+回车  

换行二：就是在中间空一行

下面是分隔符
***
---


## 图表链接
### 表格
第一列 | 第二列 | 第3列
:--- | :---:|---:
123 | 456 | 789

### 插入网络图片
![图片获取失败显示的名称](http://www.baidu.com/img/bdlogo.gif "鼠标悬停显示的名称")

### 插入本地图片
![本地图片](./pics/notexist.png)

### 插入链接
[2 Add Two Numbers](https://leetcode-cn.com/problems/add-two-numbers/submissions/ "鼠标悬停显示文字")

### 插入代码

```cpp{.line-numbers}
#include <iostream>
```

### 引用

> One man’s constant is another man’s variable. — Alan Perlis

### 锚点：跳转用的
[跳转到画图](#如何画图)

### 任务列表   
+ [x] 需求分析
+ [x] 系统设计
+ [x] 详细设计
+ [ ] 编码
+ [ ] 测试
+ [x] 交付

## 导出
方法一：选择 ebook - html, 所有本地图片都会转化为 base64 数据。

方法二：Save as  Markdown, 导出能够发布到 Github 的 markdown

## 画图
使用 markdown 来画图太复杂了，直接找好的画图工具，画完后，粘贴过来即可。

工具：ProcessOn, Draw.io Integration

## 数学公式
采用 Latex 语法，其实我很少用到数学公式，所以首选公式生成器。

### 参考文档

[Latex教程](https://blog.csdn.net/c20182030/article/details/84840373#textoperatorname_86)

[Latex在线文档](https://katex.org/docs/supported.html "先查这个文档")

[在线公式生成器](http://latex.codecogs.com/eqneditor/editor.php)

### 希腊字母

希腊字母大小写，全看首字母。  
$\alpha$ $\beta$ $\gamma$ $\lambda$
$\pi$ $\sigma$ $\theta$ 
$\delta$ $\Delta$  

### 运算符

$+$、$-$、$\times$、$\div$  
$>$、$<$  $=$ $\equiv$、$\leq$、$\geq$、$\neq$  
$\approx$  
$\bmod$
$n^3$  $\sqrt[3]{n}$  
$\frac{ab}{cd}$
$\binom{n}{k}$
$C_{2}^{2}$


$\in$ $\notin$ $\ni$  
$\subset$、$\subseteq$、$\nsubseteq$ $\supset$、$\supseteq$  
$\cup$、$\cap$ $\infty$

### Variable-sized symbols

$\sum$
$\prod$


### 标准函数

$\sin$ $\cos$ 
$\arcsin$  $\arccos$
$\tan$ $\cot$
$\max$  $\min$ 
$\log$

### 排版显示

| 表示 |作用  |  |
| --- | --- | --- |
| $$ | 独立一行 |  |
| $ | 文中公式 |  |
|$a \space b$|空格||
|$a \quad b$|一个空格，宽度接近M||
|$a \qquad b$|2个M空格||
|\\|也是换行||
|\text{}|文本内容|

### 例子

$f(x)=\frac{P(x)}{Q(x)}$

$$f(x)=\frac{P(x)}{Q(x)}$$

$$f(n)=n^n \quad \text{$n\in N^\ast$}$$

$7x \times 2y \div 7 \neq 3z$

$x^2 + \sqrt{3x-1}+\sqrt[5]{2y^5-4}$

$64 \mod 3$

$$\cos (2\theta) = \cos^2 \theta - \sin^2 \theta$$

$$\sum_{n=1}^\infty k$$

$$\frac{n!}{k!(n-k)!} = \binom{n}{k}$$

$$\lim\limits_{x \to \infty} \exp(-x) = 0$$

$$
  \begin{matrix}
   1 & 2 & 3 \\
   4 & 5 & 6 \\
   7 & 8 & 9
  \end{matrix} 
$$

$$
X(m,n)=
\begin{cases}
x(n),\\
x(n-1),\\
x(n+1)
\end{cases}
$$

