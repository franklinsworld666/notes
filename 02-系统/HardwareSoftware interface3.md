    

# Hardware/Software interface3

Section7： 

1. cache basis
2. principle of locality
3. memory hierarchies
4. cache organization
5. program optimizations that consider caches

  

1.  cache basis

1.1 how does execution time grow with SIZE

      int array[SIZE];

      int A = 0;

      for (int i = 0 ; i < 200000 ; ++ i) {

           for (int j = 0 ; j < SIZE ; ++ j) {

           A += array[j];

          }

      }

if the array fits cache,   time grows slower.

![graphic](HardwareSoftware%20interface3-20260911132012001.png)            

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

1.2 processor----memory bottleneck

1.2 cache catches a subset of the block, i.e. data is copied in block-sized, not in byte-sized

1.3 hit --- miss:  when the CPU wants data, whether it can find data in cache.

![graphic](HardwareSoftware%20interface3-20260911132012001-1.png)

  

  

  

  

  

  

  

  

  

  

  

2 principles of locality

2.1 why cache work

      programs tend to use data and instructions with addresses near or equal to those they have used recently.

      temporal locality: recently referenced items are likely to be referenced again in the near future

      spatial locality: items nearby tend to be referenced in time

  

example:  spatial: fetch more data and instructions each time

![graphic](HardwareSoftware%20interface3-20260911132012002.png)

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

3 memory hierarchies

3.1 huge difference between a hit and a miss:  could be 100X

3.2 use miss rate instead of hit rate:  3~10% for L1

3.3 hit time: time to deliver a line in the cache to the processor, typical hit times: 1 ~2 clock cycles for L1

3.4 miss penalty: additional time required because of a miss, typically 50 ~200cycles

3.5 fundamental idea of a memory hierarchy: each level k serves as a cache for the larger, slower, level k+1 below

3.6 The memory hierarchy creats a large pool of storage that costs as much as the cheap storage near the bottom, but that serves data to programs at the rate of the fast storage near the top.

3. ![graphic](HardwareSoftware%20interface3-20260911132012002-1.png)

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

Intel core i7 cache hierarchy

![graphic](HardwareSoftware%20interface3-20260911132012002-2.png)

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

4 cache organization

4.1 where should we put data in cache?

      Divide memory in blocks, put one block in cache each time

4.2 how did we know whether the data is in cache?

      Add tag, tag = block number % cache size

![graphic](HardwareSoftware%20interface3-20260911132012003.png)![graphic](HardwareSoftware%20interface3-20260911132012003-1.png)  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

4.3  problems with direct mapped caches?

Example: cache has 4 blocks, then fetch data at address 2, 6, 2, 6.....

I will miss every time.

  

4.4 If store data in any place in the cache?

That might slow down caches, so we do sth in between

first, we calculate the set number, and then we choose in the set arbitrarily

![graphic](HardwareSoftware%20interface3-20260911132012004.png)  

  

  

  

  

  

  

  

  

4.5 block replacement

for direct mapped: only one choice, it's easy.

For associative mapped: such as example2, A and B is in set3, and we want to put C in set3, so which one should we kick out? A or B? How to choose? Replace what is least recently used.

  

4.6 cache read from the memory

first, find the set

second, check if line in set

third, find the match tag

fourth, yes + line valid: hit

fifth: locate date in block

![graphic](HardwareSoftware%20interface3-20260911132012004-1.png)  

  

  

  

  

  

  

  

4. 7 cache write to the memory

write-through(write immediately to memory)

write-back( defer write to memory until line is evicted:  need a dirty bit to indicate if line is different

  

5 program optimizations that consider caches

  

  

  

Section8 process

1. jump & call return,  change control flow ------- change in program state

2. exceptions , which is tranfer of control to OS ----- change in system state, processor state

examples:  div by 0, page fault, I/O request completes

![说明: A description...](HardwareSoftware%20interface3-20260911132012004.jpg)

  

  

3. interrupt vector：表中有各类中断事件的指针。

![说明: A description...](HardwareSoftware%20interface3-20260911132012005.jpg)                                                                 

4. asynchronous exceptions----caused by events external to the processor

异步异常，由外部事件引起。

4.1 indicated by setting the processor's interrupt pin

![说明: A description...](HardwareSoftware%20interface3-20260911132012005-1.jpg)

  

  

4.2 handler returns “next” instruction

4.3 examples:  I/O interrupts,  hard reset interrupt, soft reset interrupt—ctrl-alt-delete

5. synchronous exceptions：执行指令引起

5.1 caused by events that occur as a result of executing an instruction

5.2 traps陷阱: transfer control to OS to perform some function, return to “next”

5.3 example: system calls

5.4 faults: i.e., page faults, segment protection faults, divide by 0 – re-execute current instruction or abort

5.5   aborts: machine check , parity error--- aborts current program

6. summary

![说明: A description...](HardwareSoftware%20interface3-20260911132012005-2.jpg)

进程

What is a process

1.  Exceptional control flow is the mechanism that the OS uses to enable multiple processes to run on the same system.

2.  process provides each program with two key abstractions:

2.1 logical control flow: each process seems to have exclusive use of the CPU

2.2 private virtual address space: seems to have exclusive use of main memory

3. context switching

3.1 processes are managed by the kernel

3.2 the kernel is not a separate process, but rather runs as part of a user process

4. run A, save A, restore B, run B....

  

![说明: A description...](HardwareSoftware%20interface3-20260911132012006.jpg)

  

5. Creating new processes & programs

1. fork-exec model:

1.1 fork() creates a copy of the current process

1.2 execve() replaces the current process' code & address space with the code for a different program

2. fork() & execve() are system calls