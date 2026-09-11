    

# Hardware/Software interface4

Section9: virtual memory

what problems does virtual memory solve?

l  how to fit a huge memory into a tiny physical memory

l  manage the memory spaces of multiple processes

l  protect processing from stepping on each other's memory

l  allow processes to share common parts of memory

1. Indirection

1. 假定int *p = &a; 然后我把a移到另一个地方。

在没有Indireciton的情况下，p如何引用a呢？就是每次移动a，我都要找到那些引用a的变量，修改其绝对地址。非常麻烦。

如果有Indirection，p不是直接存放a的地址，而是指向一个目录，比如目录中的第三行，在这个第三行中存放着&a，当我移动a的时候，我只要修改目录第三行的地址就可以了。

虽然这增加了空间消耗，却带来时间上的便利。

 ![](HardwareSoftware%20interface4-20260911132010855.png)

2. 在嵌入式系统中，不需要重定位，是直接取址。

3. a system using virtual addressing

CPU把指令中的虚拟地址给MMU，MMU把虚拟地址转换为主存中的真实地址

 ![](HardwareSoftware%20interface4-20260911132010856.png)

4.![](HardwareSoftware%20interface4-20260911132010856-1.png)

5. address translation

将process划分为一个个page，将内存划分为page，每次取一个page，当page离开内存时整体写回disk。

为了知道page是否在内存中，每个process都在内存里维护了一张page table，里面放有每个page的信息，是否在内存中，disk address 或是 内存的页号。

 ![](HardwareSoftware%20interface4-20260911132010857.png)

6. CPU读取虚地址，从虚地址获得page number + page offset， 根据页表基地址 + 页号 去查找页表，确定page是否在内存中。如果在，就从页表中获得了该页在内存中的页号。 

如果不在，就产生page fault——这是一个内部exception----然后把缺页从disk放到内存。如果内存满了，就需要kick page out。这涉及替换算法。

 ![](HardwareSoftware%20interface4-20260911132010857-1.png)

7. 还可以在page table增加权限信息

 ![](HardwareSoftware%20interface4-20260911132010858.png)

8. page hit：取一次页表，再取一次数据。

 ![](HardwareSoftware%20interface4-20260911132010858-1.png)

9. page fault：exception后，从disk取page放入memory&cache，并更新PT，然后从新执行指令。所以，从cache取页表1次，从disk取page1次，更新页表，取页表，取数据。

 ![](HardwareSoftware%20interface4-20260911132010859.png)

10. MMU最少需要access memory2次，为了加快速度，在MMU内增加一个cache，专门存放page table。一般可以存放128～256个page table entries。 

11. TLB hit：访问一次TLB，访问一次memory/cache获得数据。

 ![](HardwareSoftware%20interface4-20260911132010859-1.png)

12. TLB miss：MMU 从cache/memory 取得PTE并放入TLB，同时也获得了入口地址。然后从cache/memory取得数据。故需要访问一次TLB，2次cache/memory

 ![](HardwareSoftware%20interface4-20260911132010859-2.png)

Section10 dynamic memory allocation

l  动态内存分配

l  实现

l  garbage collection

l  common memory-related bug in C programming

1. Dynamic memory allocation

内存在heap分配，当返回NULL或errno时，就是unsuccessful

有一个指针指向heap顶部，确定了heap的值

 ![](HardwareSoftware%20interface4-20260911132010860.png)

1.1 types of allocators

explicit allocator: allocate and free space

implicit allocator: allocate, but does not free space. E.g. garbage collection in Java

1.2 Example

#include <stdlib.h>

void * malloc( size_t size)     //返回指针，至少分配size bytes，因为要考虑对齐。GNU malloc on linux 按8个字节对齐

void free(void *p)

1.3 performance goal

l  throughput

number of completed request per unit time

5000 malloc() calls and 5000 free() calls in 10 seconds: 1000 operations/second

l  peak memory utilization

P是一次request完成后分配的bytes，H是current heap size

注意：H是单调不减的，使用sbrk()分配

 ![](HardwareSoftware%20interface4_files/Image%20[11].png)

1.4 限制peak memory utilization的原因 – fragmentation

  

l  internal fragmentation – 你需要的size 比 block小，但是OS还是按照block分配，于是有些字节就被浪费了。

![](HardwareSoftware%20interface4-20260911132010860-1.png)

  

l  external fragmentation

2. implementation

l  how much memory to free given just a pointer

l  keep track of the free blocks

l  how do we pick a block to use for allocation (when many blocks might fit)?

l  what do we do with the extra space (when size is smaller than the free block)

l  how do we reinsert freed block into the heap

2.1 how much to free

使用一个header 存放分配的空间大小

 ![](HardwareSoftware%20interface4-20260911132010860-2.png)

2.2 keeping track of free blocks

l  implicit free lists

用一个word来存放长度，同时利用最后三位来表示是否分配。000代表free，001代表allocated。（因为是按8个字节对齐，所以word的最后3位肯定为0）

  

implicit list：finding a free block

![](HardwareSoftware%20interface4-20260911132010861.png)

  

first fit：从头找起，找到第一个满足的。

线性时间，但是会在头部造成碎片。

p = heap start;

while ( (p < end) && ( (*p & 1) || (*p <= len) ) )

    p = p + ( *p & -2);

// *p & 1获得allocated bit;    *p & -2 把最后一位掩盖

Next fit：每次从上次结束的地方开始搜索

通常比first fit 快，因为避免了重复扫描已分配的blocks

某些情况下，碎片会更严重

Best fit：找到最佳的block

碎片比较小，一般比first fit速度慢

implicit list: allocating in free block

void addblock (ptr p, int len) {

int newsize = ( (len + 1) >> 1) << 1;  // 对齐需要，偶数words

int oldsize = *p & -2;

*p = newsize | 1;   // 最后一位置1，表示分配了

if (newsize < oldsize) {

    *( p+newsize) = oldsize – newsize;   // 代码是有问题的

}

implicit list: free blocks，coalescing

第一种：

 ![](HardwareSoftware%20interface4-20260911132010861-1.png)

  

第二种：

![](HardwareSoftware%20interface4-20260911132010861-2.png)

  

l  explicit free lists

只列free blocks，但是用链表的方式，第一个word是size，第二个向后的指针，第三个向前的指针。

 ![](HardwareSoftware%20interface4_files/Image%20[17].png)

insert policy： where put the newly freed blocks

LIFO: 放在free list的表头，只需要常数时间，但是碎片问题比按地址排序严重

address-ordered policy: free blocks按地址排序，搜索时间是线性的

例子：LIFO

 ![](HardwareSoftware%20interface4-20260911132010862.png)

Comparison to implicit list:

分配在free blocks的线性时间内完成，而非全部blocks

需要额外的空间存放链接

进出free lists更加复杂

通常和segregated list连用

l  Segregated list

按大小分成几个lists，通常每个small size有个list，large size就几个共一个list

 ![](HardwareSoftware%20interface4-20260911132010862-1.png)

3. garbage collection

mark and sweep collection

在每一个可以到达的block头部，加一个mark bit。然后搜索这个block，找到这个block中的指针，由指针到达其他block，以此类推。

 ![](HardwareSoftware%20interface4-20260911132010862-2.png)