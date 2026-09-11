    

# Hardware/Software interface1

Hardware/Software interface1: section 1 ～ 4  

[Hardware/Software interface 2](HardwareSoftware%20interface%202.html)：section 5 ～ 6 （stack，

[Hardware/Software interface3](HardwareSoftware%20interface3.html)： section 7 ～ 8 （cache, exception）

[Hardware/Software interface4](HardwareSoftware%20interface4.html)：section9 ～ 10 （VM，dynamic memory alloction)

  

  

1. Data and Addressing

- how to represent data in terms of zeros and ones
- how do we organize and address data in memory
- how do we reference that data   

1.1 从memory取得指令和数据，放入寄存器，在寄存器中进行操作，最后将结果写回memory

1.2  CPU  has it's own devices for getting the next instruction.即PC

1.3 计算机的performance：CPU， memory bandwidth

1.4 strategies: increase memory bandwidth: DDR1--DDR3; cache

1.5 0：a low voltage, 0~0.5V, 1: a high voltage, 2.8~3.3V

1.6 用二进制表示一个数，但要决定用多少位来表示一个数

  

1.7 内存按照字节来组织，just a long list of bytes, 每个字节都有一个地址，根据这个地址就能找到它

1.8 machine word: 整数的长度，32-bit machine中是4bytes

1.9 一个word有4bytes， 那么word的地址是: 第一个字节的地址，

1.10 比如第一个word地址0， 下一个word就是4，

1.11 OS为每一个process分配一个私有的地址空间，程序可以访问自己的地址空间，修改自己的数据，但是不能访问其他process的地址空间

1.12 process = program + data + state

1.13 address 和 pointer 的区别：地址是一个内存中的位置，指针是一个数据对象，它的值就是地址。同样一串二进制数，我将它解释为数，还是解释为地址，这就是两者的区别。

1.14 bool变量占据1个字节，因为总是按照字节编址，所以bool变量不可能用1bit来存放bool，否则就无法给出地址了。

1.15  byte ordering：字节序。

1.16 big endian/ Sun, Internet, power pc: big end first, most significant byte has lowest address;  

1.17 little endian/ X86: little end first, least-significant byte has lowest address 小端序

![](HardwareSoftware%20interface1-20260911132011622.png)

  

  

1.18 int array[128];  int *ptr;

     ptr = array + 3;   // 首先array是地址，然后看array类型int*,故+12

     ptr = array[130];  // out of bounds, but c doesn't check

1.19 在标准的char string中，大端序、小端序没有区别

1.20 unicode使用4个bytes来表示一个字符

1.21 用bit vector代表一个集合：01101001,表示集合{0,3,5,6}

  

  

2. integer & floating point numbers

- 整数表示：有符号、无符号
- unsigned and signed integers in C
- arithmetic and shifting
- sign extension
- fractional binary numbers
- IEEE floating-point standard
- floating-point operations and rounding
- floating-point in C

2.1 编码

如何表示52张牌？ 

1. 用52个bits，每个bit代表一张牌，缺陷：难以比较大小，而且用了太多位；2. 用4bits表示花色，13位代表值，共17bits，缺陷：仍然过多； 

3. 用6bits二进制编码，缺陷：不易比较大小；

4. 用2位表示花色，用4bits表示数值，不超过1byte

  

2.2 整数表示

1. 用首尾来表示符号位sign-and-magnitude

drawbacks：存在2个0，0000 和 1000；匹配存在困难，4-3 ！= 4+（-3），0100 - 0011 ！= 0100 + （1011），必须考虑绝对值，符号位不能直接相加

  

2. 2's complement

只有1个0，直接进行加减法

～X + 1 = -X

  

2.3 integers in c

1. 整数默认都是有符号的

2. 在末尾增加后缀U表示无符号数，0U

3. 有符号数和无符号的转换：只是用 有符合 或 无符号 的方式去解释那个位串

4. 在一个表达式中混用signed 和 unsigned，会转化成unsigned

![](HardwareSoftware%20interface1-20260911132011622-1.png)

  

2.4 shifting and sign extention

1. 有符号数、无符号数、算术位移、逻辑位移

逻辑移位就是直接左右移动，补0；算术移位，右移时补符号位，左移补0

if (x) a = y; else a = z;    // x 是0 或1

改写为：a = ((x << 31) >> 31) & y + ((!x << 31)>> 31) & z;

2. sign extension

将w位的signed integer x 改为 w + k bits, keeep same value

高k位为原来的符号位，低w位不变

  

2.5 fractional binary numbers

1. 

![](HardwareSoftware%20interface1-20260911132011623.png)

2. normalized: 正规化，把M变为1.XXXX的形式

3. 既然M以1开头，那么实际上就不用去储存1

4. 特殊值的表示：

![](HardwareSoftware%20interface1-20260911132011623-1.png)

s：0/1， bit pattern 00...0:  表示0

exp == 11...1 and fract == 00...0, 表示无穷大

exp == 11...1 and fract != 00...0, 表示：Not a number // 这是一个比无穷还大/小的数，所以这不是一个数。用来表示未定义的操作：sqrt（-1） ， 无穷 - 无穷...

6. E = exp - bias， 单精度：127， 双精度：1023

所以，单精度：E（-126， 127）， 双精度（-1022， 1023）

7. 例子

![](HardwareSoftware%20interface1-20260911132011623-2.png)

  

2.6 floating-point operations

1. 浮点数运算：先做精确运算，然后rounding到IEEE 754的形式

2. 可能的问题：指数溢出，frac地位被舍去

3. rounding：round-to-even

3. 浮点运算并不总是服从结合律或分配律，由于rounding

![](HardwareSoftware%20interface1-20260911132011623-3.png)

第一个：大数 + 小数，由于rounding，3.14其实被舍掉，

第二个：右边1e40，E溢出，无穷 - 无穷 = 无穷

  

2.7 FP in c

1. default rounding model：round-to-even

2. 比较2个浮点数是否相等，不要用==， 而是做减法，然后差在允许范围内。

3. 整数和浮点数的转换--改变了位串

![](HardwareSoftware%20interface1-20260911132011624.png)

4. double、float转化为 int

小数部分可能被舍去，

可能超过int的表示范围，写出Tmin、Tmax

  

section3 machine code

- what is an ISA(instruction set architecture)
- intel处理器和体系结构简史
- c, 汇编，机器码
- x86 basics: registers

3.1 what is an ISA

1. 翻译影响程序性能 ：程序执行的时间取决于4个方面：源程序、编译器（翻译成怎样的汇编指令）、ISA（决定了编译器可以使用的指令集）、硬件。

  

2. The ISA defines: the system's state(e.g registers, memory, pc),  the instructions the CPU can execute;   每条指令对程序状态的影响

  

3. general ISA design decisions

![](HardwareSoftware%20interface1-20260911132011624-1.png)

  

4. ISA：当我们写汇编代码时，我们必须知道处理器的其中一部分设计，这部分就是ISA，也可以说：这部分对软件是透明的。比如：register的数量。

  

3.2 c， assembly，machine code

1. Assembly programmer's view

![](HardwareSoftware%20interface1-20260911132011624-2.png)

  

2. turn c into object code

源代码 -- 编译--汇编代码--汇编--机器码--链接--可执行二进制文件

链接：添加静态库，动态库运行时链接；重定位

![](HardwareSoftware%20interface1-20260911132011625.png)

  

3. g++ -O1 -S main.cpp 生成汇编代码

as main.s -o main.out  生成对象文件

ld main.out -o main.exe  生成可执行文件

objdump -d p 反汇编，即将二进制翻译成汇编代码

g++ -c main.cpp   只编译不链接

g++ main.o -o main 链接成可执行文件

  

4. 三种基本的指令：1. 在寄存器和内存间移动数据；2. 对寄存器和内存数据进行运算；  3. 条件/无条件 转移

  

3.4. registers

1. integer registers

共8个寄存器，6个通用寄存器，2个专用寄存器（专用于指针）

由于向后兼容的原因，不一定一次性存取32位，也可以16，乃至8位。

![](HardwareSoftware%20interface1-20260911132011625-1.png)

  

2. X86-64 integer registers

扩展了原有的寄存器，增加了8个新的寄存器。都可以按8位、16位、32位、64位存取

![](HardwareSoftware%20interface1-20260911132011626.png)

  

  

  

  

  

section4：X86 assembly programming

- 移动指令、寄存器、操作数
- memory addressing modes
- swap example: 32-bit vs 64-bit
- 算术运算
- 条件代码
- 条件/无条件 分支
- 循环
- switch statements

4.1 move data

movx source,dest   // x is one of (b, w, l)

movl source,dest: move 4-byte "long word"

movw source,dest: move 2-byte "word"

movb source,dest: move 1-byte "byte"

  

operand types

immediate: constant integer.  0x400, 但只能是source type

register：%eax, %edx

memory: 在寄存器中的内存地址，如%eax

![](HardwareSoftware%20interface1-20260911132011626-1.png)

  

Memory addressing mode

indirect : 在寄存器中存放内存地址

movl (%ecx),%eax

  

displacement: Mem[Reg[R]+D]

movl 8(%ebp),%edx

  

4.2 32-bit vs 64-bit

movq: move 8bytes

addq  salq  

X86-64还是能使用32位指令，寄存器的高位被设置成0，如addl

  

4.3 Memoryaddressing modes

1. D（Rb, Ri,S) ===   Mem[Reg[Rb] + S * Reg[Ri] + D]

// S 默认为1，D默认为0

0x80(, %edx, 2):  2 * %edx + 0x80

  

2. leal Src,Dest   // src 是一个地址表达式，dest存储计算结果

// lea ： load effective address

// leal (%edx,%ecx,4),%eax

  

3. two operand(binary) instructions:

![](HardwareSoftware%20interface1-20260911132011626-2.png)

  

4. one operand unary instructions:

![](HardwareSoftware%20interface1-20260911132011627.png)

  

5. 汇编代码与C代码，指令的顺序不完全一样。有些表达式需要多条汇编指令，有些表达式可以合并成一条汇编表达式

  

4.5 条件语句

1. jx 指令

![](HardwareSoftware%20interface1-20260911132011627-1.png)

2. 条件代码（隐式设定）

single-bit registers

CF carry flag (for unsigned)

SF sign flag(for signed)

ZF zero flag

OF overflow flag(for signed)

  

cmpl/cmpq src2,src1    // 计算 src1 - src2

testl/testq src2,src1  // a & b

  

SetX instructions: set a single byte to 0 or 1 based on 条件码

![](HardwareSoftware%20interface1-20260911132011627-2.png)

  

![](HardwareSoftware%20interface1-20260911132011628.png)

  

  

4.7 循环

1. for 循环，while循环

![](HardwareSoftware%20interface1-20260911132011628-1.png)