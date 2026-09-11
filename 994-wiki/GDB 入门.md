   

# GDB 入门：创建 core 文件及基本命令

系统默认不会生成 core 文件，所以一般要先进行配置。如下

```

# 查看 core 文件大小

ulimit -c

  

# 把 core 文件的上限设置为 unlimited

ulimit -c unlimited

  

# 对于 linux

# 设置 core 文件保存路径以及文件名格式，默认是在当天程序目录下生成

# %e: 程序名

# %p: 进程号

# %t: 时间

echo "./core_%e_%p_%t" > /proc/sys/kernel/core_pattern

  

# 对于mac， core 文件在 /core 下面
```

  


程序加`-g` 编译，运行，并手动杀死它，观察是否产生了 core 文件。

如果没有产生core文件，考虑是否对信号进行了处理。

```

g++ -g main.cpp -o target

  

./target &

  

kill -11 targetPid

  

gdb target corefile

```
  

## 基本命令

- gdb ./bomb        // 调试 bomb 程序
    
- bt                          // 通常用来显示死机时的栈帧
    
- info frame          // 显示当前栈帧
    
- f  frame_num    // 跳到某一帧
    
- info threads          // 显示线程
    
- thread thread_id    // go to a specific thread
    
- info registers
    
- p var                        // print the value of var 
    
- p /x var                    // 以十六进制显示 var
    
- x /2w 0x80            // 显示2个words，从地址0x80开始
    
- x /128bx  0x80    // 显示 128 个字节
    
- x /2wx $rsp           // 从%rsp开始的2个word， 用十六进制显示
    
- disas 0x80         // 反汇编地址0x80附近的程序
    
- disas sum          // 反汇编 程序sum 
    
- info line *addr              // 根据地址，显示对应的代码行
    
- info symbol addr          // 根据地址，显示对应的符号
    

  

## 其他命令

- break sum           // 在程序sum入口设置断点
    
- break *0x80         // 在地址0X80设置断点
    
- r                         // 运行程序，如果有参数可以跟在后面
    
- stepi                  // 执行一条指令
    
- stepi 4              // 执行4条指令
    
- nexti                  // 执行下一条指令，而且穿过函数调用--即把函数调用视为一条指令
    
- continue            // 继续执行
    
- finish                // 执行，直到当前程序返回
    
- print /x ($rsp+8） // 打印%rsp+8的内容，以十六进制
    
- watch                        // 设置观察点，当值变化时，显示
    
- p *data@7               // {70 30  1  7  15 20 50}