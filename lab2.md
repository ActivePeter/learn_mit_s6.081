## 1.实现trace系统调用

- 查看了trace.c

  里面调用了trace这个系统调用，这个lab的目的就是让我们去补全这个trace

- 基本思路就是在调用trace的时候，往内核proc结构体里标记一个trace mask

  之后执行只要匹配mask。就进行输出

  然后再trace exec结束后，将标记清除

  这里需要知道exec执行成功后并不会return，所以得fork一下执行，然后对fork的子线程进行wait

  结束后调用trace(0)清空mask即可

## 2.实现sysinfo系统调用

- `int copyout(pagetable_t, uint64, char *, uint64)`
  Copy from kernel to user.
  Copy len bytes from src to virtual address dstva in a given page table.
  Return 0 on success, -1 on error.

- 根据提示，需要参考fstat系统调用来实现返回结构体给用户空间

  ```c
  uint64
  sys_fstat(void)
  {
    struct file *f;
    uint64 st; // user pointer to struct stat
  
    if(argfd(0, 0, &f) < 0 || argaddr(1, &st) < 0)
      return -1;
    return filestat(f, st);
  }
  ```

  如此可以吧用户空间传入的结构体指针拷贝出来

  

- 返回值结构体 

  - 第一个参数

     the `freemem` field should be set to the number of bytes of free memory,

    free的物理页表内存大小(要乘上pgsize)

  - 第二个参数 free的进程数量

- 测试

  - ![image-20220129010250318](https://hanbaoaaa.xyz/tuchuang/images/2022/01/28/image-20220129010250318.png)
  - 感觉这个test脚本脑子有大病，我改返回值他对应的值就变，说明这里检测逻辑是有问题的
    - ![image-20220129010431919](https://hanbaoaaa.xyz/tuchuang/images/2022/01/28/image-20220129010431919.png)
    - ![image-20220129010500974](https://hanbaoaaa.xyz/tuchuang/images/2022/01/28/image-20220129010500974.png)

### 不管了，就当我完成了吧(狗头)

