---
title: Tcpdump 基本用法
tags: 
notebook: 1102 程序开发
---

[TOC]

tcpdump 是一个linux下的抓包工具，基于libpcap。基础的命令如下：
```
tcpdump -i interface proto-dir-type-filterExpr -w savefile
```
其中 `-i` 后面的参数是网络接口，`-w` 后面是要保存的文件名。  

整个抓包命令的关键，就是写一个过滤表达式，拿到你需要的包。  
tcpdump 还有很多其他命令，但我基本不用所以本文不介绍。

## 基于场景的过滤表达式
过滤表达式的一般规则：
+ 包括：协议、方向、类型三个元素
+ 协议：tcp、udp、arp、ip、icmp 等
+ 方向：src、dst、src and dst、src or dst 等
+ 类型：地址、端口等

### 基于协议过滤
+ 抓 tcp/udp/arp/ip 包
```
tcpdump -i eth0 tcp/udp/arp/ip -w savefile
```

### 基于方向过滤
+ 抓目的端口是80的 tcp 包
```
tcpdump -i any tcp dst port 80 -w savefile
```

### 基于 IP 地址过滤
+ 抓本机和主机 A 之间的包  
```
tcpdump -i eth0 host A -w savefile
```

+ 抓源地址是主机 A 的包  
```
tcpdump -i eth0 src host A -w savefile
```

+ 抓目的地址是主机 A 的包
```
tcpdump -i eth0 dst host A -w savefile
```

+ 抓某一个子网络的包
```
tcpdump -i eth0 net A -w savefile
```

### 基于端口过滤
+ 抓源端口是 80 的包
```
tcpdump -i eth0 src port 80 -w savefile
```

+ 抓目的端口是 80 的包
```
tcpdump -i eth0 dst port 80 -w savefile
```

+ 抓端口 sport 和 dport 之间的包
```
tcpdump -i eth0 src port sport and dst port dport -w savefile
```

+ 抓端口范围 6000 - 8000 的包
```
tcpdump -i eth0 portrange 6000-8000 -w savefile
```

### 显示信息，但不保存成文件
```
tcpdump -i eth0 -nnvvX
tcpdump -i any -nvvX 
tcpdump -i eth0 icmp -nnvvX
```


### 其他
+ 查看过滤表达式帮助文档
```
man pcap-filter
```

+ 指定网络接口/任意接口
```
tcpdump -i eth0
tcpdump -i any
```

+ 找出可以抓包的interface， 比如：本机的回环 lo 或者其他 
```
tcpdump -D
```

+ 限制从一个包中截取的字节数
```
# 针对每一个包，截取开头多少个字节
# 目的是：限制抓包大小，提高后续分析效率；
# 分析网络问题，很多时候只要一个包头就够了，内容是不需要的
tcpdump -s 80
#截取所有
tcpdump -s 0
```

+ 查看丢包数量
```
# tcpdump 抓包结束后，会显示有多少收到的包被 drop，往往是因为 tcpdump buffer 不够
# 解决方案：增加过滤条件，或者限制每个包的大小
XXX packets dropped by kernel
```

+ 如果抓包文件太大，不好解析怎么办
```
# 每抓 100MB 保存成一个文件
tcpdump -i bond0 host 192.168.10.42 -C 100 -Z root -w zhuabao.pcap

# 每抓 60S 保存成一个文件
tcpdump -i bond0 host 192.168.10.42 -G 60 -Z root -w %Y_%m%d_%H%M_%S.pcap

# 把包切分成多个小包
tcpdump -r zhuabao.pcap -w newfile -C packetSize
```

+ 其他参数
```
-nn: 不解析主机名或端口名
-X: 将数据包以16进制和ascii格式显示
-v,-vv,-vvv: 不断增加显示的信息
-c: 捕获特定数量的包
-S：打印绝对序列号
```
