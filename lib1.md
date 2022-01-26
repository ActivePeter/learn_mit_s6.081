- 第一个lab，看起来挺简单的

  - 然而第一步make qemu就bug了

    ***

    *** Error: Couldn't find a riscv64 version of GCC/binutils.
    *** To turn off this error, run 'gmake TOOLPREFIX= ...'.

    ***

    发现是上面配置环境那步漏看了

    [6.S081 / Fall 2020 (mit.edu)](https://pdos.csail.mit.edu/6.828/2020/tools.html)

    ### Installing via APT (Debian/Ubuntu)

    Make sure you are running either "bullseye" or "sid" for your debian version (on ubuntu this can be checked by running `cat /etc/debian_version`), then run:

    ```
    sudo apt-get install git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu 
    ```

    (The version of QEMU on "buster" is too old, so you'd have to get that separately.)

    #### qemu-system-misc fix

    At this moment in time, it seems that the package `qemu-system-misc` has received an update that breaks its compatibility with our kernel. If you run `make qemu` and the script appears to hang after
    `qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0`
    you'll need to uninstall that package and install an older version:

    ```
      $ sudo apt-get remove qemu-system-misc
      $ sudo apt-get install qemu-system-misc=1:4.2-3ubuntu6
    ```

  - ```c
    int fork() Create a process, return child’s PID.
    int exit(int status) Terminate the current process; status reported to wait(). No return.
    int wait(int *status) Wait for a child to exit; exit status in *status; returns child PID.
    int kill(int pid) Terminate process PID. Returns 0, or -1 for error.
    int getpid() Return the current process’s PID.
    int sleep(int n) Pause for n clock ticks.
    int exec(char *file, char *argv[]) Load a file and execute it with arguments; only returns if error.
    char *sbrk(int n) Grow process’s memory by n bytes. Returns start of new memory.
    int open(char *file, int flags) Open a file; flags indicate read/write; returns an fd (file descriptor).
    int write(int fd, char *buf, int n) Write n bytes from buf to file descriptor fd; returns n.
    int read(int fd, char *buf, int n) Read n bytes into buf; returns number read; or 0 if end of file.
    int close(int fd) Release open file fd.
    int dup(int fd) Return a new file descriptor referring to the same file as fd.
    int pipe(int p[]) Create a pipe, put read/write file descriptors in p[0] and p[1].
    int chdir(char *dir) Change the current directory.
    int mkdir(char *dir) Create a new directory.
    int mknod(char *file, int, int) Create a device file.
    int fstat(int fd, struct stat *st) Place info about an open file into *st.
    int stat(char *file, struct stat *st) Place info about a named file into *st.
    int link(char *file1, char *file2) Create another name (file2) for the file file1.
    int unlink(char *file) Remove a file.
    ```

    

  - 第一步是写一个sleep服务

    - 我先复制一个echo，改成echo2试试

      ![image-20220125222437792](https://hanbaoaaa.xyz/tuchuang/images/2022/01/25/image-20220125222437792.png)

      在UPROGS种加上echo2

      make qemu

      echo2指令生效了，和echo一个效果

    - 然后是实现 sleep

      思路一开始就对了，问题主要花在了调试上，每次改完都要重开一遍..

  - 2.pingpong

    - 折磨了很久。总算行了，

    - ![image-20220126000042162](https://hanbaoaaa.xyz/tuchuang/images/2022/01/25/image-20220126000042162.png)

    - 主要问题是对这个系统调用不熟，fork返回若是child，返回的是0，若是parent 返回 child的pid

    - 原进程3，4是子进程

      ![image-20220126000336013](https://hanbaoaaa.xyz/tuchuang/images/2022/01/25/image-20220126000336013.png)

  - 3.并发计算素数

    ![image-20220126002450059](https://hanbaoaaa.xyz/tuchuang/images/2022/01/25/image-20220126002450059.png)

    大概原理就是这个图

    A generating process can feed the numbers 2, 3, 4, ..., 1000 into the left end of the pipeline: the first process in the line eliminates the multiples of 2, the second eliminates the multiples of 3, the third eliminates the multiples of 5, and so on:

    第一个进程遍历所有数，并将过滤出的结果发给第二个进程。

    后一个进程以渠道的第一个值为被除数过滤

    - 一个经常犯得问题，取参数取了0，应该取下标为1的
    - pipe创建要在fork之前，不然就变成创建两个了

