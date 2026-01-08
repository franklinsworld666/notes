---
title: Linux 常用命令
tags:  
notebook: 1102 程序开发 
---

[TOC]

Linux 命令那么多，怎么才能记住这么多命令？方法如下：
+ 重要命令多用，熟能生巧
+ 次要的命令，查找历史记录或者自己的笔记
+ 以上不行，就 bing 搜索并记录到笔记中

## 进程相关
+ 显示 CPU 使用信息

`top` 显示并不断更新进行信息，包括每个进程信息，默认按照 CPU 使用率排序。  

`1:` 显示每个 CPU 的统计信息  

`M:` 按照内存占用排序 

`load average:` 分别是过去 1min、5min、15min 的负荷

`f:` 可以调整显示的列

+ 显示进程信息
```
// 这个命令有 Unix BSD GNU 这三种参数风格。
// 记住常用的几个，按风格顺序排列：
ps -elfH
ps alfx
``` 

+ 显示系统内存
```
// 显示系统内存信息，并每3秒刷新一下
// total: 总的内存
// used:  已经使用的
// free:  空闲内存
// shared：被共享使用的内存
// buff/cache:  被磁盘缓存占用的，可被释放
// available: 还可以被程序使用的内存
// 粗略估计，available = free + buff/cache
free -h -s 3

// 类似的命令
cat /proc/meminfo
```


+ 返回指定进程的 ID
```
pidof init
```

+ 杀死进程  
```
kill -11 pid
kill -s KILL pid  

// 杀死进程及其子进程，或杀死同名进程
pkill pidname
killall -11 pidname
```

+ 列出当前进程所有 fd  
```
ls -l /proc/pid/fd
```

+ 显示进程打开的文件
```shell
lsof -p $$
```

+ 显示后台进程   
```
jobs
```

+ 信号
```
// 显示所有信号
kill -l 

// 了解各种信号如何产生
man signal
```

+ 显示程序执行时间
```
// real  user  sys
// 一般，real != user + sys
time ls -l 
```

+ 显示程序依赖的共享库
```
ldd /bin/ls 
```

## 网络
+ 显示网络连接情况
```
netstat -pantu

// 显示连接队列，RECV-Q current connect len, SEND-Q 配置的队列长度
ss -tln
```

+ 查看网络统计信息
```
netstat -s 
```

+ 显示组播组信息
```
netstat -g 
```

+ arp 地址 arp, arping
```shell
# 查看缓冲区
arp -an
arp -ni eth0

# 设定 IP 和 MAC 的静态映射
arp -s ip MAC

# 从 ARP 表删除
arp -d ip

## 测试是否有 IP 地址冲突，同网段有效
arping -D -I eth0 172.16.1.1
```


+ 探测网络是否通
```
// 收到3个应答就终止
ping -c 3 www.baidu.com

// 禁止 IP 层分包来测试网络的 MTU
ping 192.168.1.1 -s 1472 -M do
ping 192.168.1.1 -s 1473 -M do

// win 上测试 MTU
ping 192.168.1.1 -l 1472 -f
```

+ ip 命令
```
# ifconfig netstat route 等命令早就不维护了，建议使用 ip 命令 

# 基本命令格式
# OBJECT : link 网卡 address 地址 neighbour 邻居表 route 路由表
#          rule ip策略 maddress 多播地址 tunnel ip隧道
# command：add delete show
ip [option] OBJECT {command | help}

## 显示网络地址
ip a
ip addr show eth0
ip addr add 172.16.213.91/16 dev eth0 
ip addr del 172.16.213.91/16 dev eth0 
### 给网卡再添加一个 ip , 可以当虚拟 ip 用 
ip addr add 192.168.0.20/24 dev eth0 label eth0:1


## 查看 arp 表
ip n
ip neigh show  
### 添加静态 arp
ip neigh add 192.168.0.106 lladdr MAC dev eth0 nud perm
ip neigh del 192.168.0.106 dev eth0 


## 路由规则
### rule 具有优先级，数字越小优先级越高；rule 策略会先于路由表执行
### ip rule [list | add | del | flush ] SELECTOR ACTION
### SELECTOR: [not][from PREFIX][to PREFIx][fwmark FWMARK][iif string][oif string][pref num]
### ACTION:   [table {local | main | default | number}][prohibit | reject]
ip rule add from 172.16.1.1/16 table 200
ip rule add fwmark 2 table 200
ip route add default via 192.168.1.1 dev eth0 table 200
### 查看
ip rule


## 查看、设置路由地址
### 默认路由：走 eth0, gateway 192.168.2.1 destnetwork 192.168.2.7/16
ip r 
ip route add default via 192.168.2.1 dev eth0  
ip route add 192.168.2.7/16 via 192.168.2.1 dev eth0  
### 删除路由
ip route del default    
ip route del 192.168.2.0/24  

### 获取特定 IP 的路由信息
ip route get 172.16.1.1
### 清空路由表
ip route flush  

## 显示网卡上的数据包统计信息
### errors: 错误包
### dropped: 由于内存不够等因素，拷贝到内存时被丢
### overrun: 数据没到 ringbuffer 被丢
### mcast: 组播包数
ip -s -s link show eth0


## 网卡属性设置
ip link set eth0 up
ip link set eth0 mtu 1500
ip link set eth0 address MAC  
```


+ 查看和设置网卡
```
# 获取网卡设置信息
ethtool eth1

# 查网卡的驱动信息、固件信息
ethtool -i eth1

# 查看网卡收发的统计信息
ethtool -S eth1

# 查看网卡接收/发送 ring 参数
ethtool -g eth1
# 设置 ring 参数
ethtool -G eth1 rx 4096

# 查看 offload 设置
ethtool -k eth1
# 设置 offload 参数
ethtool -K eth1 GRO off

# 设置网口速率10/100/1000M、网口半/全双工、网口是否自协商
# ethtool –s ethX [speed 10|100|1000] [duplex half|full]  [autoneg on|off] 
ethtool -s eth0 speed 100 duplex full autoneg off

# 查看网卡中接收RX、发送TX和Autonegotiate模块的状态：启动on 或 停用off。
ethtool -a eth0

# 修改网卡中RX、TX和Autonegotiate的状态：启动on 或 停用off。
ethtool -A tx on/off eth0

# 显示聚合网口信息，使看起来更有规律。
ethtool -c 
```




## 查找
+ which 

在PATH变量指定的路径中，搜索某个系统命令的位置，并且返回第一个搜索结果  
```
which grep
```

+ locate  

配合数据库查看文件位置  
```
locate /etc/sh
```

+ 搜寻硬盘查找文件  
```
// find <指定目录> <指定条件> <指定动作>
// exec：对匹配的文件执行该参数给出的shell命令 
// xargs 主要承接删除操作

// 删除 最后一次修改 距今15天前的XXX文件
find ./test -type f -name "*.log$" -mtime +15 -exec rm {} \; 
find . -type f -name "*.log$" -mtime +15 | xargs rm -f

// find 的有用参数
// -name : 文件名
// -type ：文件类型
// -or   ：或运算
// -maxdepth: 递归最大深度 
find ./ -name "test" -or -name "love" -maxdepth 2
find ./ -name "*test*" -type f -exec ls -al {} \;
```

+ 查找文件内容
```
// grep [option] pattern file
// -n ：显示匹配的行号
// -r ：递归查找
// -v : 显示不匹配的结果
// -c : 匹配的数量
// -i ：忽略字符大小写
// -P : Perl 风格正则匹配

// 多个匹配
grep -rnP "mp3|mp4" ./*

// 进程数量
ps afx|grep "irqbalance" |grep -v "grep" -c
```

## 系统信息
+ 显示系统信息，包括内核版本  
```
uname -a
```

+ 查看 CPU 信息
```
// physical ：物理 CPU 个数
// CPU cores：每个 CPU 的有多少个核
// processors：逻辑核数，如果支持超线程，一个物理核可以当成几个逻辑核用
cat /proc/cpuinfo
```

+ 显示和加载模块
```
# 显示模块
lsmod

# 加载模块
# depmod 产生模块依赖的映射文件，由modprobe使用
# modprobe 比 insmod 更好，因为还能处理依赖关系
depmod -a
modprobe p.ko 
```

+ 显示开机信息
```
dmesg | less
```

+ 查看启动日志
```
# centos 才有
journalctl -r

```

+ 显示系统启动时间
```
# current time
# the time the system has been operational
# the load
uptime
```

+ 显示和修改重要的系统参数
```shell
# 显示所有变量
sysctl -a

# 临时修改变量
sysctl -w kern.maxfiles=5000

# 永久生效
修改 /etc/sysctl.conf
sysctl -p
```

+ 系统服务管理命令
```
systemctl status nginx 
systemctl restart nginx
systemctl stop nginx

# start while booting
systemctl enable nginx
systemctl disable nginx
```


+ 查看设备关机重启记录
```
last | grep reboot
```

+ 监视某个内容变化
```shell
# -d 高亮差异，-n 间隔
watch -d -n 10 "cat /proc/net/snmp | grep -w Udp"
```

## 文件系统
+ 递归创建目录
```
// 如果 AAA 不存在，就创建
mkdir -p AAA/BBB
```

+ 安装或升级软件
```
// 实质就是把文件拷贝到某个目录
install -D a/b/myfile c/d/destfile
```

+ 复制文件，并保留文件属性
```
# 保持文件属性不变，比如时间属性
cp -p file1 file2

# 复制符号链接时，把目标也复制为符号链接
cp -d file1 file2

# cp -a == cp - dpr
cp -a dir1 dir2
```


+ 创建软连接
```
ln -s sourcefile softlink
```

+ 远程拷贝文件
```
scp source dest 
```

+ 判断硬盘是否存在
```shell
fdisk -l /dev/sda1 > /sd
if [ -s /sd ];then
    mount /dev/sda1 /media
fi
```

+ 挂载
```
// 查看外设名称
dmesg 
lsblk

// 查看已挂载设备
mount

// 挂载到电脑
// filesystem: vfat, ntfs, nfs etc
// option : ro, rw etc
mount -t filesystem -o option device directory

// /dev/sda1 
sd : scsi 接口硬盘
hd : IDE 接口硬盘
abcd: 第几块硬盘
1234: 主分区
56789: 扩展分区/逻辑分区

// 把远程设备上的一个目录挂载在本地设备
// 远程设备要开启 NFS 服务
mount -t nfs -o rw 10.140.133.9:/home/sunky /mnt/nfs 

// 查看挂载的进程, 杀掉他们从而强制卸载
fuser -mv /media
lsof /media

// 卸载
umount /mnt/usbhd1
```


+ 查看磁盘使用
```
# 磁盘使用情况
df -h

# 查看目录占用的空间
du -sh path

# 查看某个目录及其每个文件占用的空间
du -h path
```

+ 压缩与解压缩
```
// 打包，因为很多压缩软件之支持压缩一个文件
tar -cvf yasuo.tar file1 file2 

// 只查看包里的文件，不解包
tar -tvf yasuo.tar

// 解包
tar -xvf yasuo.tar -C ./

// 打包，同时压缩
// -z:gz
// -j:bz2
tar -cvzf yasuo.tar.gz file1 file2

// 解压，解包
tar -xvzf yasuo.tar.gz -C ./
```

+ 建立或还原备份档
```shell
# 还原
cpio -iv < /etc/etc.img

# 创建
find . | cpio -ov -H newC > /etc/etc.img
```

+ 查看文件类型
```
file /bin/mv
```

## 文本处理
+ 查看文件开始或结尾多少行
```
head -n 50
tail -n 50
less XXX
```

+ 监控文件变化
```
tail -f XXX
```

+ 统计文本行数等
```shell
// 字节数, words, lines
ls a.txt | wc -c
ls a.txt | wc -w 
ls a.txt | wc -l 
```

+ 从文本中拿到某一块
```shell
// 行内第3个字节
cat /etc/passwd | cut -b 3

// 行业第3个字符
cat /etc/passwd | cut -c 3

// 用:做分割符，第2块内容
cat /etc/passwd | cut -d : -f 2
```

+ 切割文件为多个
```shell
# 切割文件大小10k，采用3位数字后缀
# dstfile001, dstfile002
split -b 10k srcfile dstfile -d -a 3
```

## 其他
+ 搜寻历史命令
```
history | grep keyword

// 利用终端提供的功能
ctrl + r
```

+ 关于 find 命令几个时间
```
// 更改时间，-n指n天以内，+n指n天以前
-mtime   -n +n
// 访问时间
-atime    -n +n
// 创建时间
-ctime    -n +n
```

+ 查看谁登陆
```
w
```


+ 起一个别名，方便使用
```shell
alias ll='ls -alh'
```

+ 不打印日志
```
ls > /dev/null
```

+ 修改换行符
```
# -k 保持时间戳不变
dos2unix -k file
```

+ 查看启动脚本
```
cat /lib/systemd/system/rc.local.service
```


