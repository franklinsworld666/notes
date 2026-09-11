    

# Hardware/Software interface 2

section5： procedures & stacks

- stacks in memory and stack operations
- the stack used to keep track of procedure calls
- return addresses and return values
- stack-based languages
- the linux stack frame
- passing arguments on the stack
- allocating local variables on the stack
- register-saving conventions
- procedures and stacks on X64 architecture

  

1. stacks in memory and stack operations

![](HardwareSoftware%20interface%202-20260911132011365.png)

  

![](HardwareSoftware%20interface%202-20260911132011365-1.png)

  

1.1 stack地址是向下增长的，有一个专门的寄存器%esp，指向stack最小的地址

  

1.2 call stack：Push

pushl src : 从src取得值，%esp减少4，将src放入%esp指向的位置

popl Dest

  

  

2.  the stack used to keep track of procedure calls

![](HardwareSoftware%20interface%202-20260911132011365-2.png)

2.1 procedure call: call labe1

%esp向下增加

push return address on stack: 返回地址是call之后的指令地址

jump to labe1：%eip修改为被调用函数地址

  

2.2 procedure call: ret

pop return address from stack

jump to address

  

3 return values

3.1 返回值是放在%eax中的

3.2 所以在调用函数之前，要先保存%eax的值

3.3 返回值是4个字节的，如果大于4个字节，最好返回指针

  

4. stack-based language

4.1 支持递归的语言

4.2 代码必须可重入——静态的指令可以同时有多个实例

4.3 每一个实例都要保存状态——返回地址，局部变量，参数

  

5. the linux stack frame

![](HardwareSoftware%20interface%202-20260911132011366.png)

call_swap() { swap(int & yp, int & xp);}

![](HardwareSoftware%20interface%202-20260911132011366-1.png)

  

以上是boby之后的收尾阶段。

第一句：restore %ebx

第二句：%esp 指向 old%ebp

第三句：将old%ebp 放入 %ebp， %esp指向Rtn adr

注意：2、3两句可以合并为一句：leave

第4句： 将adr放入%eip，%esp指向xp

![](HardwareSoftware%20interface%202-20260911132011366-2.png)

注意：保存和恢复了%ebx，但是对于其他寄存器如%eax, %ecx, %edx没有这么做。

  

  

7. register-saving conventions

caller-save: save before calling

callee-save: save before using

special form of callee: save - restored to original values upon exit from procedure

 ![](HardwareSoftware%20interface%202-20260911132011367.png)

  

  

![](HardwareSoftware%20interface%202-20260911132011367-1.png)

这里有3个frame

1. 放x，放返回地址，

2. 保存原来的的%ebp，%ebp成为第2个frame的结尾

3. 给第2个frame 分配一个临时的空间，这里是16个bytes

4. 把x放入%edx

5. local变量val放入临时空间

![](HardwareSoftware%20interface%202-20260911132011367-2.png)

  

  

![](HardwareSoftware%20interface%202-20260911132011368.png)

  

9. procedures and stacks on X64 architecture

9.1 因为寄存器数量翻倍了，所以，对stack依赖就变小了

9.2 store 参数、临时变量 in registers

![](HardwareSoftware%20interface%202-20260911132011368-1.png)

  

64位swap程序：没有stack operations

![](HardwareSoftware%20interface%202-20260911132011369.png)

  

一个程序所占用的内存区域，可分为代码区、全局数据区、栈区、堆。代码区存放指令，函数都在代码区；全局变量存放在全局数据区，局部变量存放于栈区，动态内存分配从堆取得内存空间。

  

Stack空间到底有多大？

这与OS有关：

1. linux中由系统指定，ulimit -a查看stack大小，ulimit -s 32768指定stack为32MB

2. windows中储存在可执行文件中，由连接程序ld确定，gcc -Wl,--stack=16777216