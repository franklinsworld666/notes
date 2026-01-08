---
title: meminfo 详解
tags: 
notebook: 1101 程序设计
---

[TOC]


负责输出/proc/meminfo的源代码是：
fs/proc/meminfo.c : meminfo_proc_show()
```
MemTotal:        3809036 kB
MemFree:          282012 kB
MemAvailable:     865620 kB
Buffers:               0 kB
Cached:           854972 kB
SwapCached:       130900 kB
Active:          1308168 kB
Inactive:        1758160 kB
Active(anon):    1010416 kB
Inactive(anon):  1370480 kB
Active(file):     297752 kB
Inactive(file):   387680 kB
Unevictable:           0 kB
Mlocked:               0 kB
SwapTotal:       4063228 kB
SwapFree:        3357108 kB
Dirty:                 0 kB
Writeback:             0 kB
AnonPages:       2104412 kB
Mapped:            40988 kB
Shmem:            169540 kB
Slab:             225420 kB
SReclaimable:     134220 kB
SUnreclaim:        91200 kB
KernelStack:        5936 kB
PageTables:        35628 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:     5967744 kB
Committed_AS:    5626436 kB
VmallocTotal:   34359738367 kB
VmallocUsed:      351900 kB
VmallocChunk:   34359363652 kB
HardwareCorrupted:     0 kB
AnonHugePages:    139264 kB
HugePages_Total:       0
HugePages_Free:        0
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
DirectMap4k:      204484 kB
DirectMap2M:     3915776 kB
```


## MemTotal
系统从加电开始到引导完成，firmware/BIOS要保留一些内存，kernel本身要占用一些内存，最后剩下可供kernel支配的内存就是MemTotal。这个值在系统运行期间一般是固定不变的。可参阅解读DMESG中的内存初始化信息。
```
### 举例
Memory: 14830376K/16768064K available (9569K kernel code,
8915K rwdata, 3416K rodata, 280K init, 2791K bss,
1937688K reserved, 0K cma-reserved)
```

## MemFree
表示系统尚未使用的内存。[MemTotal-MemFree]就是已被用掉的内存。

## MemAvailable
系统中有些内存虽然已被使用但是可以回收的，比如 cache/buffer、slab都有一部分可以回收，所以这部分可回收的内存加上MemFree才是系统可用的内存，即 MemAvailable。

/proc/meminfo 中的 MemAvailable 是内核使用特定的算法估算出来的，要注意这是一个估计值，并不精确。

有些应用程序会根据系统的可用内存大小自动调整内存申请的多少，所以需要一个记录当前可用内存数量的统计值，MemFree并不适用，因为MemFree不能代表全部可用的内存。


## 内存都到哪里去了？
### 内存黑洞
追踪Linux系统的内存使用一直是个难题，很多人试着把能想到的各种内存消耗都加在一起，kernel text、kernel modules、buffer、cache、slab、page table、process RSS…等等，却总是与物理内存的大小对不上，这是为什么呢？<font color=red>因为Linux kernel并没有滴水不漏地统计所有的内存分配。</font>

Kernel的动态内存分配通过以下几种接口：

+ alloc_pages/__get_free_page

以页为单位分配，不会自动统计

+ vmalloc

以字节为单位分配虚拟地址连续的内存块。通过vmalloc分配的内存也有统计，参见/proc/meminfo中的VmallocUsed 和 /proc/vmallocinfo

+ slab allocator

kmalloc: 以字节为单位分配物理地址连续的内存块，它是以slab为基础的，使用slab层的general caches — 大小为2^n，名称是kmalloc-32、kmalloc-64等。

通过slab层分配的内存会被精确统计，可以参见/proc/meminfo中的slab/SReclaimable/SUnreclaim；


### 内核
内核所用内存的静态部分，比如内核代码、页描述符等数据在引导阶段就分配掉了，并不计入MemTotal里，而是算作Reserved(在dmesg中能看到)。

内核所用内存的动态部分，是通过上文提到的几个接口申请的，其中通过alloc_pages申请的内存有可能未纳入统计，就像黑洞一样。

下面讨论的都是/proc/meminfo中所统计的部分。

#### SLAB
通过slab分配的内存被统计在以下三个值中：

+ SReclaimable: slab中可回收的部分。调用kmem_getpages()时加上SLAB_RECLAIM_ACCOUNT标记，表明是可回收的，计入SReclaimable，否则计入SUnreclaim。

+ SUnreclaim: slab中不可回收的部分。

+ Slab: slab中所有的内存，等于以上两者之和。

#### VmallocUsed
通过vmalloc分配的内存都统计在/proc/meminfo的 VmallocUsed 值中，但是要注意这个值不止包括了分配的物理内存，还统计了VM_IOREMAP、VM_MAP等操作的值，譬如VM_IOREMAP是把IO地址映射到内核空间、并未消耗物理内存，所以我们要把它们排除在外。

从物理内存分配的角度，我们只关心VM_ALLOC操作，这可以从/proc/vmallocinfo中的vmalloc记录看到：
```
grep vmalloc /proc/vmallocinfo | awk '{total+=$2}; END {print total}'
23375872
```

一些driver以及网络模块和文件系统模块可能会调用vmalloc，加载内核模块(kernel module)时也会用到，可参见 kernel/module.c。

#### kernel modules
kernel module所占用的内存包含在/proc/vmallocinfo的统计之中，不必再去计算”lsmod”的结果了，而且”lsmod”也不准。

#### HardwareCorrupted
当系统检测到内存的硬件故障时，会把有问题的页面删除掉，不再使用，/proc/meminfo中的HardwareCorrupted统计了删除掉的内存页的总大小。相应的代码参见 mm/memory-failure.c: memory_failure()。

#### PageTables
Page Table用于将内存的虚拟地址翻译成物理地址，随着内存地址分配得越来越多，Page Table会增大，/proc/meminfo中的PageTables统计了Page Table所占用的内存大小。

注：请把Page Table与Page Frame（页帧）区分开，物理内存的最小单位是page frame，每个物理页对应一个描述符(struct page)，在内核的引导阶段就会分配好、保存在mem_map[]数组中，mem_map[]所占用的内存被统计在dmesg显示的reserved中，/proc/meminfo的MemTotal是不包含它们的。（在NUMA系统上可能会有多个mem_map数组，在node_data中或mem_section中）。


#### KernelStack
每一个用户线程都会分配一个kernel stack（内核栈），内核栈虽然属于线程，但用户态的代码不能访问，只有通过系统调用(syscall)、自陷(trap)或异常(exception)进入内核态的时候才会用到，也就是说内核栈是给kernel code使用的。在x86系统上Linux的内核栈大小是固定的8K或16K。

Kernel stack（内核栈）是常驻内存的，既不包括在LRU lists里，也不包括在进程的RSS/PSS内存里，所以我们认为它是kernel消耗的内存。统计值是/proc/meminfo的KernelStack。

#### Bounce
有些老设备只能访问低端内存，比如16M以下的内存，当应用程序发出一个I/O 请求，DMA的目的地址却是高端内存时（比如在16M以上），内核将在低端内存中分配一个临时buffer作为跳转，把位于高端内存的缓存数据复制到此处。这种额外的数据拷贝被称为“bounce buffering”，会降低I/O 性能。大量分配的bounce buffers 也会占用额外的内存。

### 用户进程
/proc/meminfo统计的是系统全局的内存使用状况，单个进程的情况要看/proc/<pid>/下的smaps等等。

#### Mapped
上面提到的用户进程的file-backed pages就对应着/proc/meminfo中的”Mapped”。

Page cache中(“Cached”)包含了文件的缓存页，其中有些文件当前已不在使用，page cache仍然可能保留着它们的缓存页面；

而另一些文件正被用户进程关联，比如shared libraries、可执行程序的文件、mmap的文件等，这些文件的缓存页就称为mapped。”Mapped”是”Cached”的子集。

因为Linux系统上shared memory & tmpfs被计入page cache(“Cached”)，所以被attached的shared memory、以及tmpfs上被map的文件都算做”Mapped”。


#### Cached
Page Cache里包括所有file-backed pages，统计在/proc/meminfo的”Cached”中。

Cached是”Mapped”的超集，就是说它不仅包括mapped，也包括unmapped的页面，当一个文件不再与进程关联之后，原来在page cache中的页面并不会立即回收，仍然被计入Cached，还留在LRU中，但是 Mapped 统计值会减小。【ummaped = (Cached – Mapped)】

Cached包含tmpfs中的文件，POSIX/SysV shared memory，以及shared anonymous mmap。


#### Buffers
“Buffers”表示块设备(block device)所占用的缓存页，包括：直接读写块设备、以及文件系统元数据(metadata)比如SuperBlock所使用的缓存页。它与“Cached”的区别在于，”Cached”表示普通文件所占用的缓存页。

“Buffers”所占的内存同时也在LRU list中，被统计在Active(file)或Inactive(file)。

#### Mlocked
“Mlocked”统计的是被mlock()系统调用锁定的内存大小。被锁定的内存因为不能pageout/swapout，会从Active/Inactive LRU list移到Unevictable LRU list上。也就是说，当”Mlocked”增加时，”Unevictable”也同步增加，而”Active”或”Inactive”同时减小；当”Mlocked”减小的时候，”Unevictable”也同步减小，而”Active”或”Inactive”同时增加。

“Mlocked”并不是独立的内存空间，它与以下统计项重叠：LRU Unevictable，AnonPages，Shmem，Mapped等。