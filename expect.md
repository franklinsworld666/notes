---
title: expect
tags:  
notebook: 1102 程序开发
---

[TOC]

## 介绍
expect 用来实现自动的交互式任务，无需人为干预。在实际开发中，可以帮助我们自动登录远程设备、执行某些命令。

## 安装
```shell
sudo apt-get update
sudo apt-get install expect
```

## 常用命令
+ spawn       	启动进程
+ expect       	从进程接收字符串，获取匹配信息，匹配成功则执行expect后面的程序动作
+ send          用于发送指定的字符串信息，模拟用户的输入，注意一定要加回车 \r
+ exp_continue  多次匹配就需要用到
+ send_user     用来打印输出 相当于shell中的echo
+ exit          退出expect脚本
+ eof           expect执行结束 退出
+ set           定义变量
+ puts          输出变量
+ set timeout   设置超时时间，-1不超时
+ interact      用户交互
+ -d   : 开启调试模式 expect -d << EOF 


## 远程登录获取内存信息
```shell
#!/bin/bash
set -euxo pipefail

user=root
ip="172.16.0.1"
passwd="123456\r"

expect << EOF
	set timeout 5
	spawn ssh $user@$ip
	expect {
		"yes/no" {send "yes\r";exp_continue}
		"password" {send $passwd}
	}
	expect "]#" {send "free -h\r"}
	expect "]#" {send "\[ \$? -eq 0 ] && echo ~~~~~~~~success~~~~~~~~ || echo ~~~~~~~~fail~~~~~~~~\r"}
	send "exit\r"
	expect eof
EOF
```


## 拷贝脚本到远程设备，并执行
```shell
#!/bin/bash
set -euxo pipefail

user=root
ip="172.16.0.1"
passwd="123456\r"

expect << EOF
	set timeout 5
	spawn scp ./yyy.sh $user@$ip:/home
	expect {
		"yes/no" {send "yes\r";exp_continue}
		"password" {send $passwd}
	}
	expect "100%"
	expect eof

	spawn ssh $user@$ip
	expect {
		"yes/no" {send "yes\r";exp_continue}
		"password" {send $passwd}
	}
	expect "]#" {send "/home/yyy.sh\r"}
	expect "]#" {send "\[ \$? -eq 0 ] && echo ~~~success~~~ || echo ~~~fail~~~\r"}
	expect eof
EOF
```
