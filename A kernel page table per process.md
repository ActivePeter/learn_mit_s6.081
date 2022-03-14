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

  - 大概意思是本来kernel内无法直接访问用户空间的指针，这里要实现在内核态直接解引用用户的指针

  - 分析

    - Some hints:

      - Add a field to `struct proc` for the process's kernel page table. x

      - A reasonable way to produce a kernel page table for a new process is to implement a modified version of `kvminit` that makes a new page table instead of modifying `kernel_pagetable`. You'll want to call this function from `allocproc`.

        **kvminit**:内核页表初始化 x

        要创建一个类似内核页表初始化的函数，用于初始化进程用户页表

      - Make sure that each process's kernel page table has a mapping for that **process's kernel stack**. In unmodified xv6, all the kernel stacks are set up in `procinit`. You will need to move some or all of this functionality to `allocproc`.

        进程页表需要有对内核栈的映射 x

      - Modify `scheduler()` to load the process's kernel page table into the core's `satp` register (see `kvminithart` for inspiration). Don't forget to call `sfence_vma()` after calling `w_satp()`.

        修改进程调度来实现切换进程是，切换进程页表到satp

      - `scheduler()` should use `kernel_pagetable` when no process is running.

        没有进程执行时，页表应换会内核页表

      - Free a process's kernel page table in `freeproc`.

        - ```c++
          
          
          // Recursively free page-table pages.
          // All leaf mappings must already have been removed.
          void freewalk(pagetable_t pagetable)
          {
            // there are 2^9 = 512 PTEs in a page table.
            for (int i = 0; i < 512; i++)
            {
              pte_t pte = pagetable[i];
              if ((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0)
              {
                // this PTE points to a lower-level page table.
                uint64 child = PTE2PA(pte);
                freewalk((pagetable_t)child);
                pagetable[i] = 0;
              }
              else if (pte & PTE_V)
              {
                panic("freewalk: leaf");
              }
            }
            kfree((void *)pagetable);
          }
          ```

      - You'll need a way to free a page table without also freeing the leaf physical memory pages.

      - `vmprint` may come in handy to debug page tables.

      - It's OK to modify xv6 functions or add new functions; you'll probably need to do this in at least `kernel/vm.c` and `kernel/proc.c`. (But, don't modify `kernel/vmcopyin.c`, `kernel/stats.c`, `user/usertests.c`, and `user/stats.c`.)

      - A missing page table mapping will likely cause the kernel to encounter a page fault. It will print an error that includes `sepc=0x00000000XXXXXXXX`. You can find out where the fault occurred by searching for `XXXXXXXX` in `kernel/kernel.asm`.

    - kernel version

      ```c
      void kvminit()
      {
        kernel_pagetable = (pagetable_t)kalloc();//物理page
        memset(kernel_pagetable, 0, PGSIZE); //4096
      
        // uart registers
        kvmmap(UART0, UART0, PGSIZE, PTE_R | PTE_W);
      
        // virtio mmio disk interface
        kvmmap(VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);
      
        // CLINT
        kvmmap(CLINT, CLINT, 0x10000, PTE_R | PTE_W);
      
        // PLIC
        kvmmap(PLIC, PLIC, 0x400000, PTE_R | PTE_W);
      
        // map kernel text executable and read-only.
        kvmmap(KERNBASE, KERNBASE, (uint64)etext - KERNBASE, PTE_R | PTE_X);
      
        // map kernel data and the physical RAM we'll make use of.
        kvmmap((uint64)etext, (uint64)etext, PHYSTOP - (uint64)etext, PTE_R | PTE_W);
      
        // map the trampoline for trap entry/exit to
        // the highest virtual address in the kernel.
        kvmmap(TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);
      }
      
      // add a mapping to the kernel page table.
      // only used when booting.
      // does not flush TLB or enable paging.
      void kvmmap(uint64 va, uint64 pa, uint64 sz, int perm)
      {//最终调用的是map pages来将虚拟地址映射到物理地址上，通过页表
        if (mappages(kernel_pagetable, va, sz, pa, perm) != 0)
          panic("kvmmap");
      }
      // Create PTEs for virtual addresses starting at va that refer to
      // physical addresses starting at pa. va and size might not
      // be page-aligned. Returns 0 on success, -1 if walk() couldn't
      // allocate a needed page-table page.
      int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
      {
        uint64 a, last;
        pte_t *pte;
      
        a = PGROUNDDOWN(va);
        last = PGROUNDDOWN(va + size - 1);
        for (;;)
        {
          if ((pte = walk(pagetable, a, 1)) == 0)
            return -1;
          if (*pte & PTE_V)
            panic("remap");
          *pte = PA2PTE(pa) | perm | PTE_V;
          if (a == last)
            break;
          a += PGSIZE;
          pa += PGSIZE;
        }
        return 0;
      }
      ```

    - initproc allocproc

      ```c
      void
      procinit(void)
      {
        struct proc *p;
        
        initlock(&pid_lock, "nextpid");
        for(p = proc; p < &proc[NPROC]; p++) {
            initlock(&p->lock, "proc");
      
            // Allocate a page for the process's kernel stack.
            // Map it high in memory, followed by an invalid
            // guard page.
            char *pa = kalloc();
            if(pa == 0)
              panic("kalloc");
            uint64 va = KSTACK((int) (p - proc));
            kvmmap(va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
            
            p->kstack = va;
        }
        kvminithart();
      }
      
      // Switch h/w page table register to the kernel's page table,
      // and enable paging.
      void kvminithart()
      {
        w_satp(MAKE_SATP(kernel_pagetable));
        sfence_vma();
      }
      
      // Look in the process table for an UNUSED proc.
      // If found, initialize state required to run in the kernel,
      // and return with p->lock held.
      // If there are no free procs, or a memory allocation fails, return 0.
      static struct proc*
      allocproc(void)
      {
        struct proc *p;
      
        for(p = proc; p < &proc[NPROC]; p++) {
          acquire(&p->lock);
          if(p->state == UNUSED) {
            goto found;
          } else {
            release(&p->lock);
          }
        }
        return 0;
      
      found:
        p->pid = allocpid();
      
        // Allocate a trapframe page.
        if((p->trapframe = (struct trapframe *)kalloc()) == 0){
          release(&p->lock);
          return 0;
        }
      
        // An empty user page table.
        p->pagetable = proc_pagetable(p);
        if(p->pagetable == 0){
          freeproc(p);
          release(&p->lock);
          return 0;
        }
      
        // Set up new context to start executing at forkret,
        // which returns to user space.
        memset(&p->context, 0, sizeof(p->context));
        p->context.ra = (uint64)forkret;
        p->context.sp = p->kstack + PGSIZE;
      
        return p;
      }
      ```

  - 实现：

    - Some hints:

      - Add a field to `struct proc` for the process's kernel page table. x

        ![image-20220214171546086](https://hanbaoaaa.xyz/tuchuang/images/2022/02/14/image-20220214171546086.png)

      - A reasonable way to produce a kernel page table for a new process is to implement a modified version of `kvminit` that makes a new page table instead of modifying `kernel_pagetable`. You'll want to call this function from `allocproc`.

        **kvminit**:内核页表初始化 x

        要创建一个类似内核页表初始化的函数，用于初始化进程用户页表

        ```c++
        pagetable_t kvminit_process()
        {
          pagetable_t pt = (pagetable_t)kalloc();
          memset(pt, 0, PGSIZE);
        
          //TODO: handle err
          // uart registers
          mappages(pt,UART0,PGSIZE,UART0, PTE_R | PTE_W);
        
          // virtio mmio disk interface
          mappages(pt,VIRTIO0, PGSIZE,VIRTIO0, PTE_R | PTE_W);
        
          // CLINT
          // kvmmap(CLINT, CLINT, 0x10000, PTE_R | PTE_W);
        
          // PLIC
          mappages(pt,PLIC, 0x400000,PLIC, PTE_R | PTE_W);
        
          // map kernel text executable and read-only.
          mappages(pt,KERNBASE, (uint64)etext - KERNBASE,KERNBASE, PTE_R | PTE_X);
        
          // map kernel data and the physical RAM we'll make use of.
          mappages(pt,(uint64)etext, PHYSTOP - (uint64)etext,(uint64)etext, PTE_R | PTE_W);
        
          // map the trampoline for trap entry/exit to
          // the highest virtual address in the kernel.
          mappages(pt,TRAMPOLINE, PGSIZE,(uint64)trampoline, PTE_R | PTE_X);
        
          return pt;
        }
        ```

        在alloc proc里

        ![image-20220214172523088](https://hanbaoaaa.xyz/tuchuang/images/2022/02/14/image-20220214172523088.png)

      - Make sure that each process's kernel page table has a mapping for that **process's kernel stack**. In unmodified xv6, all the kernel stacks are set up in `procinit`. You will need to move some or all of this functionality to `allocproc`.

        进程页表需要有对内核栈的映射 x

      - Modify `scheduler()` to load the process's kernel page table into the core's `satp` register (see `kvminithart` for inspiration). Don't forget to call `sfence_vma()` after calling `w_satp()`.

        修改进程调度来实现切换进程是，切换进程页表到satp

        ![image-20220214172728323](https://hanbaoaaa.xyz/tuchuang/images/2022/02/14/image-20220214172728323.png)

      - `scheduler()` should use `kernel_pagetable` when no process is running.

        没有进程执行时，页表应换会内核页表

      - Free a process's kernel page table in `freeproc`.

        

      - You'll need a way to free a page table without also freeing the leaf physical memory pages.

      - `vmprint` may come in handy to debug page tables.

      - It's OK to modify xv6 functions or add new functions; you'll probably need to do this in at least `kernel/vm.c` and `kernel/proc.c`. (But, don't modify `kernel/vmcopyin.c`, `kernel/stats.c`, `user/usertests.c`, and `user/stats.c`.)

      - A missing page table mapping will likely cause the kernel to encounter a page fault. It will print an error that includes `sepc=0x00000000XXXXXXXX`. You can find out where the fault occurred by searching for `XXXXXXXX` in `kernel/kernel.asm`.

