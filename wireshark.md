---
title: wireshark 入门
tags: 
notebook: 1102 程序开发 
---

[TOC]

## 抓包
+ 限制抓包范围

限制抓包范围，可以减少网络包对内存的占用、对 IO 的消耗、后期的分析工作量。在 `capture --- options` 中，填写过滤表达式：`<protocol> <dir> <net/port/host/portrange>`  

例子：
```
tcp dst port 443  
not arp  
src host 192.168.1.1 and dst port 88 

# 过滤 mac
ether host 00:88:11:22:33:44
ether src host XX:XX:XX:XX:XX:XX
```

+ 数据包的保存策略

在 `capture options` 中，可以按照抓包数量、抓包大小、抓包时间来自动保存数据包。

+ 怎么保存 packet 的应用层数据（去掉协议头）


## 过滤

+ 方法一: 采用表达式
+ 方法二: 选中要过滤的内容 --- 右键 --- `Prepare a Filter`
+ 方法三：点击`Expression` 按钮，通过选中来构造表达式

常用的表达式：
```
# PDU in TCP/IP Model
# bit,frame,packet,segment

# 过滤 ip
ip.dst_host = 1.1.1.1
ip.src_host = 1.1.1.1
ip.addr = 1.1.1.1

# 过滤端口
tcp.srcport
tcp.dstport
tcp.port

# 过滤帧长
frame.len > 80

# 过滤 TCP 报文的特殊标识位
选中标志位，应用 prepare as a Filter

# 过滤协议
http
!http


```



## 统计信息
+ 流追踪

选中某个 segment，right-click --- Follow XXX Stream

+ Expert Info

Analyze --- ExpertInfo, wireshark 提供了自动分析出的 error，warning，可以重点关注一下这些信息。

+ 统计摘要

Statistics --- Summary，统计了数据流的基本信息，包括：每秒平均包数，每包平均大小，每秒平均流量等等。

+ 分层统计

Statistics --- Hierarchy, 统计了数据流中不同协议的包的占比。

+ 网络会话

Statistics --- Conversations, 按照会话统计了源和目的之间的信息。

+ 包长度统计

Statistics --- packet lengths, 统计数据流中不同长度的包的占比。

+ IO 统计

Statistics --- IO Graph

## 问题排查
+ 查找数据包

ctrl + F，然后用十六进制或字符串进行查找，ctrl+n 下一个，ctrl+b 上一个



## 其他
+ 自动展开 packet details

View --- Expand All

+ 增加新的一列用来展示信息

在 packet list 界面的标题栏，右键 --- column preferences  

+ 以某个包的时间为基准

选中包，right-click --- set time reference

+ packet list 的颜色

View --- coloring rules

+ 强制采用某种协议解析报文

选中报文，右键 --- Decode As

+ wireshark 支持解析的协议

Analyze --- Enables Protocols
