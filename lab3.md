- ![image-20220204003047277](https://hanbaoaaa.xyz/tuchuang/images/2022/02/03/image-20220204003047277.png)

- 物理内存地址是56bit，其中44bit是物理page号（**PPN**，Physical Page Number），剩下12bit是offset完全继承自虚拟内存地址（也就是地址转换时，**只需要将虚拟内存中的27bit翻译成物理内存中的44bit的page号**，剩下的12bitoffset直接拷贝过来即可）

  ```c
  // The risc-v Sv39 scheme has three levels of page-table
  // pages. A page-table page contains 512 64-bit PTEs.
  // A 64-bit virtual address is split into five fields:
  //   39..63 -- must be zero.  63-39=24
  //   30..38 -- 9 bits of level-2 index.  3*9=27
  //   21..29 -- 9 bits of level-1 index.
  //   12..20 -- 9 bits of level-0 index.
  //    0..11 -- 12 bits of byte offset within the page.
  
  #define PGSIZE 4096 // bytes per page
  #define PGSHIFT 12  // bits of offset within a page
  
  #define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
  #define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))
  
  #define PTE_V (1L << 0) // valid
  #define PTE_R (1L << 1)
  #define PTE_W (1L << 2)
  #define PTE_X (1L << 3)
  #define PTE_U (1L << 4) // 1 -> user can access
  
  // shift a physical address to the right place for a PTE.
  #define PA2PTE(pa) ((((uint64)pa) >> 12) << 10)  //物理地址转pte
  
  #define PTE2PA(pte) (((pte) >> 10) << 12)  //由pte转为物理地址
  
  #define PTE_FLAGS(pte) ((pte) & 0x3FF)  //前10位标志
  
  // extract the three 9-bit page table indices from a virtual address.
  #define PXMASK          0x1FF // 9 bits
  #define PXSHIFT(level)  (PGSHIFT+(9*(level)))
  #define PX(level, va) ((((uint64) (va)) >> PXSHIFT(level)) & PXMASK)
  
  // one beyond the highest possible virtual address.
  // MAXVA is actually one bit less than the max allowed by
  // Sv39, to avoid having to sign-extend virtual addresses
  // that have the high bit set.
  #define MAXVA (1L << (9 + 9 + 9 + 12 - 1))
  
  typedef uint64 pte_t;
  typedef uint64 *pagetable_t; // 512 PTEs
  
  ```

  

  

- **SATP** 寄存器会指向最高一级的page directory的物理内存地址

- **va** 虚拟内存地址

  ![image-20220204172701427](https://hanbaoaaa.xyz/tuchuang/images/2022/02/04/image-20220204172701427.png)

- 一个**directory**是4096Bytes，就跟page的大小是一样的  

  - 包含512个条目（一个8byte）
  - 对应类型 **pagetable_t（uint64 *  数组）**

- Directory中的一个**条目** pte:Page Table Entry 64bit   

  - 对应类型：**pte_t uint64**
  - ![image-20220204005151580](https://hanbaoaaa.xyz/tuchuang/images/2022/02/03/image-20220204005151580.png)
  - 数字为0的pte用于连接三级table
    - 在最高级，你需要一个page directory。在这个page directory中，你需要一个数字是0的PTE，指向中间级page directory。所以在中间级，你也需要一个page directory，里面也是一个数字0的PTE，指向最低级page directory。所以这里总共需要3个page directory（也就是3 * 512个条目）。
  - 

- **页表缓存（Translation Lookaside Buffer** tlb

  - 减少读内存代价

- kernal page table

- walk函数

  - ```c
    // Return the address of the PTE in page table pagetable
    // that corresponds to virtual address va.  If alloc!=0,
    // create any required page-table pages.
    //
    // The risc-v Sv39 scheme has three levels of page-table
    // pages. A page-table page contains 512 64-bit PTEs.
    // A 64-bit virtual address is split into five fields:
    //   39..63 -- must be zero.            
    //   30..38 -- 9 bits of level-2 index. 
    //   21..29 -- 9 bits of level-1 index.
    //   12..20 -- 9 bits of level-0 index.
    //    0..11 -- 12 bits of byte offset within the page.
    pte_t *
    walk(pagetable_t pagetable, uint64 va, int alloc) //satp指向最高的pagetable
    {
      if (va >= MAXVA)
        panic("walk");
    
      for (int level = 2; level > 0; level--)
      {
        pte_t *pte = &pagetable[PX(level, va)];//通过px 从va虚拟地址取出对应level的pte 
        if (*pte & PTE_V)
        {
          pagetable = (pagetable_t)PTE2PA(*pte); //pte指向下一个pagetable
        }
        else
        {
          if (!alloc || (pagetable = (pde_t *)kalloc()) == 0)
            return 0;
          memset(pagetable, 0, PGSIZE);
          *pte = PA2PTE(pagetable) | PTE_V;
        }
      }
      return &pagetable[PX(0, va)];
    }
    
    ```

  - 

- vmprint:

  ![image-20220206232400865](https://hanbaoaaa.xyz/tuchuang/images/2022/02/06/image-20220206232400865.png)

  ![image-20220206232423631](https://hanbaoaaa.xyz/tuchuang/images/2022/02/06/image-20220206232423631.png)

  有两个fail不知道咋整了，

- ## A kernel page table per process

  - 实验目的

    了解页表，实现内核态访问用户态指针

    xv6中内核使用单个页表，然后每个进程维护各自的用户态页表

    这样进入内核时，无法直接访问用户态指针，因为页表切换到了内核页表（satp），内核页表中并没有用户态的信息，必须要绕弯子访问

    

    ## 分析

    - #### 实验目的

      了解页表，实现内核态访问用户态指针

      xv6中内核使用单个页表，然后每个进程维护各自的用户态页表

      这样进入内核时，无法直接访问用户态指针，因为页表切换到了内核页表（satp），内核页表中并没有用户态的信息，必须要绕弯子访问

      

      - ##### Some hints:

      - 1.Add a field to `struct proc` for the process's kernel page table. x

      - 2。A reasonable way to produce a kernel page table for a new process is to implement a modified version of `kvminit` that makes a new page table instead of modifying `kernel_pagetable`. You'll want to call this function from `allocproc`.

      - 分析：
  
      - 实现维护每个进程专门的内核页表
  
      - 3.Make sure that each process's kernel page table has a mapping for that **process's kernel stack**. In unmodified xv6, all the kernel stacks are set up in `procinit`. You will need to move some or all of this functionality to `allocproc`.
  
      - 分析：原本xv6的做法是在procinit中申请一页作为进程 内核栈 kernel stack
  
      - 每一个进程都有一个对应的内核栈
  
      - ![image-20220314123334448](https://hanbaoaaa.xyz/tuchuang/images/2022/03/14/image-20220314123334448.png)
  
      - ![image-20220314123401630](https://hanbaoaaa.xyz/tuchuang/images/2022/03/14/image-20220314123401630.png)
  
      - 而新的模型中，因为进程切换会进入对应页表，如果内核栈依然是公共页表的va，那么可能出现往进程内核页表访问公共页表va的情况
  
      - 4.Modify `scheduler()` to load the process's kernel page table into the core's `satp` register (see `kvminithart` for inspiration). Don't forget to call `sfence_vma()` after calling `w_satp()`.
  
      - 修改进程调度来实现切换进程是，切换进程页表到satp
  
      - `scheduler()` should use `kernel_pagetable` when no process is running.
  
      - 没有进程执行时，页表应换会内核页表
  
      - 5.Free a process's kernel page table in `freeproc`.
  
      - You'll need a way to free a page table without also freeing the leaf physical memory pages.
  
      - vmprint may come in handy to debug page tables.
  
      - It's OK to modify xv6 functions or add new functions; you'll probably need to do this in at least kernel/vm.c and kernel/proc.c. (But, don't modify kernel/vmcopyin.c, kernel/stats.c, user/usertests.c, and user/stats.c.)
  
      - A missing page table mapping will likely cause the kernel to encounter a page fault. It will print an error that includes sepc=0x00000000XXXXXXXX. You can find out where the fault occurred by searching for XXXXXXXX in kernel/kernel.asm.
  
    - #### 实现
  
      1.修改进程结构体，加入进程内核页表变量
  
      2.编写进程内核页表创建函数，（基本和原来的内核页表一样）
  
      3.修改proc相关的，去除原本procinit时申请内核栈
  
      在allocproc（创建进程处）
  
      调用2写好的页表创建函数
  
      创建内核栈
  
      ![image-20220314123523375](https://hanbaoaaa.xyz/tuchuang/images/2022/03/14/image-20220314123523375.png)
  
    - 4.进程调度处，通过改变stap切换页表
  
      ![image-20220314123736856](https://hanbaoaaa.xyz/tuchuang/images/2022/03/14/image-20220314123736856.png)
  
    - 5.释放进程的用户页表以及内核页表
  
      ​	kfree stack需要指向stack物理地址，所以我在allocproc结尾加上了对pa的记录
  
      ![image-20220314123803893](https://hanbaoaaa.xyz/tuchuang/images/2022/03/14/image-20220314123803893.png)
  
    - 释放内核栈内存，解除页表映射
  
      ![image-20220314123828379](https://hanbaoaaa.xyz/tuchuang/images/2022/03/14/image-20220314123828379.png)

