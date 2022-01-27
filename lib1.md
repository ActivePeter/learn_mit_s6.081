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

    

  - ### 第一步是写一个sleep服务

    - 我先复制一个echo，改成echo2试试

      ![image-20220125222437792](https://hanbaoaaa.xyz/tuchuang/images/2022/01/25/image-20220125222437792.png)

      在UPROGS种加上echo2

      make qemu

      echo2指令生效了，和echo一个效果

    - 然后是实现 sleep

      思路一开始就对了，问题主要花在了调试上，每次改完都要重开一遍..

  - ### 2.pingpong

    - 折磨了很久。总算行了，

    - ![image-20220126000042162](https://hanbaoaaa.xyz/tuchuang/images/2022/01/25/image-20220126000042162.png)

    - 主要问题是对这个系统调用不熟，fork返回若是child，返回的是0，若是parent 返回 child的pid

    - 原进程3，4是子进程

      ![image-20220126000336013](https://hanbaoaaa.xyz/tuchuang/images/2022/01/25/image-20220126000336013.png)

  - ### 3.并发计算素数

    ![image-20220126002450059](https://hanbaoaaa.xyz/tuchuang/images/2022/01/25/image-20220126002450059.png)

    大概原理就是这个图

    A generating process can feed the numbers 2, 3, 4, ..., 1000 into the left end of the pipeline: the first process in the line eliminates the multiples of 2, the second eliminates the multiples of 3, the third eliminates the multiples of 5, and so on:

    第一个进程遍历所有数，并将过滤出的结果发给第二个进程。

    后一个进程以渠道的第一个值为被除数过滤

    - 一个经常犯得问题，取参数取了0，应该取下标为1的
    - pipe创建要在fork之前，不然就变成创建两个了
    - pipe创建时要做一个死循环等待
    - ！！！最大的问题：pipe 通信，fork之后相当于被两个线程占有，close的时候得在两个线程中都关闭一下

    

  - ### 4.find

    - 具体我写到博客里了
    - 执行步骤
      - 1.打开
      - 2.获取状态stat
      - 3.分文件夹和文件
        - 1.文件 输出
        - 2.文件夹 
          - 路径长度限制
    - 关键函数
      - fstat
        -  2、函数int fstat(int fd, struct stat *buf);
                – 参数fd：文件描述符
                – 参数*buf：文件信息
                – 返回值：成功为0，否则为-1
        - 提示：本函数与 stat() 函数相似，不同的是，它是作用于已打开的文件指针而不是文件名。
      - stat
        -  1、函数int stat(const char *path, [struct](https://so.csdn.net/so/search?q=struct&spm=1001.2101.3001.7020) stat *buf);
                – 参数*path：文件路径
                – 参数*buf：文件信息
        - ​      – 返回值：成功为0，否则为-1
      - memmove
        - 就是strcpy，安全点
      - dirent
        - 从文件得fd里读取的时候，这个是一个单位结构
    - 这些都理解之后实现还是挺简单的，就一个递归即可，
    - 测试
      - mkdir a
      - mkdir a/a
      - mkdir a/b
      - mkdir b
      - mkdir b/b
      - ![image-20220127160138336](https://hanbaoaaa.xyz/tuchuang/images/2022/01/27/image-20220127160138336.png)

    

  - ### xargs

    - 理解|含义

      利用Linux所提供的管道符“|”将两个命令隔开，管道符左边命令的输出就会作为管道符右边命令的输入。连续使用管道意味着第一个命令的输出会作为 第二个命令的输入，第二个命令的输出又会作为第三个命令的输入，依此类推。下面来看看管道是如何在构造一条Linux命令中得到应用的。

      1.利用一个管道
      \# rpm -qa|grep licq

      这条命令使用一个管道符“|”建立了一个管道。管道将rpm -qa命令的输出（包括系统中所有安装的RPM包）作为grep命令的输入，从而列出带有licq字符的RPM包来。
      4.利用多个管道

      \# cat /etc/passwd | grep /bin/bash | wc -l

      这条命令使用了两个管道，利用第一个管道将cat命令（显示passwd文件的内容）的输出送给grep命令，grep命令找出含有“/bin /bash”的所有行；第二个管道将grep的输出送给wc命令，wc命令统计出输入中的行数。这个命令的功能在于找出系统中有多少个用户使用bash。

    - 查看xv6手册 关键词 standard input

      - 找到一个 wc例子，就跟这个题目有关系

      - ![image-20220127165650324](https://hanbaoaaa.xyz/tuchuang/images/2022/01/27/image-20220127165650324.png)

      - 运行wc的例子发现，他就是对|（pipeline的输入进行了分析），所以这题直接参照wc即可

      - xargs 后面跟的内容为要执行的命令，所以我们要确定执行命令的方法

        - 在翻找sh.c时大概知道 exec函数

        - Use `fork` and `exec` to invoke the command on each line of input. Use `wait` in the parent to wait for the child to complete the command.

          题目提示中也有相关的

        - 这里我比较担心exec传入的参数列表结尾没有0的截止，所以写了一段代码测验一下

          - ```c
            int i = 0;
                char **a =
                    argv;
                //argv;
                while (*a)
                {
                    printf("%d\n", i);
                    // argv++;/
                    a++;
                    i++;
                }
            ```

            ![image-20220127194659183](https://hanbaoaaa.xyz/tuchuang/images/2022/01/27/image-20220127194659183.png)

            由结果可知结尾是有0终止的，这也意味着我们后续封装cmd不需要携带长度

      - 封装Cmd

        - ```c
          typedef struct
          {
              char **args;
              /* data */
          } Cmd;
          
          Cmd Cmd_make(char *args[])
          {
              Cmd cmd;
              cmd.args = args;
              return cmd;
          }
          
          char Cmd_exec_in_child_process(Cmd cmd)
          {
              if (cmd.args == 0 || *cmd.args == 0)
              {
                  return 0; //invalid failed
              }
              int pid = fork();
              if (pid == 0)
              {
                  exec(cmd.args[0], cmd.args);
                  exit(0);
              }
              wait(0);
          
              return 1;
          }
          ```

        - ![image-20220127201259786](https://hanbaoaaa.xyz/tuchuang/images/2022/01/27/image-20220127201259786.png)

        - 判断有效比较关键，需要考虑指针(char**)为0或者是指针第一个元素(char *)为0

      - echo hh | xargs echo aa

        ![image-20220127205033855](https://hanbaoaaa.xyz/tuchuang/images/2022/01/27/image-20220127205033855.png)

        发现结果不是预期的，有换行，

        后来才意识到这个意思是把上一个的输出作为这一个的参数输入，也就是cmd序列需要动态的变化，所以不能使用一开始函数传入的那个cmd数组

      - 重新封装

        ```c
        //////////////////////////////////////////////
        // Cmd
        
        #define Cmd_max_arg_cnt (10)
        typedef struct
        {
            char *args[Cmd_max_arg_cnt + 1];
            uint8 arg_cnt;
            /* data */
        } Cmd;
        void Cmd_print(Cmd cmd);
        Cmd Cmd_make(char *args[])
        {
            Cmd cmd;
            memset(&cmd, 0, sizeof(Cmd));
            {
                // int i = 0;
                while (cmd.arg_cnt < Cmd_max_arg_cnt && args[cmd.arg_cnt])
                {
                    cmd.args[cmd.arg_cnt] = args[cmd.arg_cnt];
                    cmd.arg_cnt++;
                }
                cmd.args[cmd.arg_cnt] = 0;
            }
            // cmd.args = args;
            return cmd;
        }
        
        Cmd Cmd_append_make(Cmd cmd, char **append)
        {
            while (cmd.arg_cnt < Cmd_max_arg_cnt && *append)
            {
                // printf("-----------append\n");
                // Cmd_print(cmd);
                // printf("-----------------\n");
                cmd.args[cmd.arg_cnt] = *append;
                append++;
                cmd.arg_cnt++;
            }
            cmd.args[cmd.arg_cnt] = 0;
            return cmd;
        }
        Cmd Cmd_append_one_make(Cmd cmd, char *append)
        {
            char *arr[2];
            arr[0] = append;
            arr[1] = 0;
            return Cmd_append_make(cmd, arr);
        }
        void Cmd_print(Cmd cmd)
        {
            printf("----------------\ncmd arg cnt %d\n", cmd.arg_cnt);
            //debug
            // {
            int i = 0;
            char **a =
                cmd.args;
            //argv;
            while (*a)
            {
                printf("%s ", *a);
                // argv++;/
                a++;
                i++;
            }
            printf("\n----------------\n");
            /////////////////////////////////////////////////////////
        }
        char Cmd_exec_in_child_process(Cmd cmd)
        {
            if (cmd.args == 0 || *cmd.args == 0)
            {
                printf("invalid cmd\n");
                return 0; //invalid failed
            }
            int pid = fork();
            if (pid == 0)
            {
                exec(cmd.args[0], cmd.args);
                exit(0);
            }
            wait(0);
        
            return 1;
        }
        ```

        以上相当于将命令片段存到了char*数组里

        append操作可以在中途进行命令的组合

        - 比如要实现

        - ```
            $ echo "1\n2" | xargs -n 1 echo line
              line 1
              line 2
          ```

          我们可以把echo line 存在一个cmd里，然后将echo输出的1,2分别append到这个cmd上，生产一个新的cmd

        Cmd_exec_in_child_process 可以创建进程并安全的在执行命令

        Cmd_print 可以方便的调试查看cmd内容

      - 然后是大循环读取部分

        - 思路是每次读取一个字节，

          - 碰到一个换行或空格则完成了一个分块的获取

            - 此时判断模式

              - 若为            if (mode == Mode_EachLine) 

                 那么就是将新片段接入cmd1(这个cmd1不能覆盖cmd，因为cmd后续还需要使用)，

                然后如果为换行，就需要执行cmd1，执行完之后，将cmd1赋值cmd，回到最初状态

              - 若为 oneshot。那就是不断将片段接入原cmd，在循环结束后再执行

        - ```c
          if (buf == '\n' || buf == '*' || buf == ' ')
          {
              vc.buff[vc_i] = 0;
              vc_i++;
              // vc_i = 0;
              if (mode == Mode_EachLine)
              {
                  if (buf == '\n' || buf == '*')
                  {
          
                      cmd1 = Cmd_append_one_make(cmd1, &vc.buff[begin]);
                      // Cmd_print(cmd1);
                      Cmd_exec_in_child_process(cmd1);
                      cmd1 = cmd;
                  }
                  else if (vc_i - begin > 1)
                  {
                      cmd1 = Cmd_append_one_make(cmd1, &vc.buff[begin]);
                  }
              }
              // vc_i = 0;
              else if (mode == Mode_OneShot)
              {
                  cmd = Cmd_append_one_make(cmd, &vc.buff[begin]);
                  // Cmd_print(cmd);
              }
              begin = vc_i;
          }
          else //common word
          {
              vc.buff[vc_i] = buf;
              vc_i++;
          }
          ```

      - 最后效果

        - 此处方便起见，将*代替换行符输入，另外，这个结尾我没有输 * 因为其默认就会有一个换行符

        - 换行模式(Mode_EachLine)

          ![image-20220127230638064](https://hanbaoaaa.xyz/tuchuang/images/2022/01/27/image-20220127230638064.png)

        - 非换行模式(Mode_OneShot)

          ![image-20220127230814325](https://hanbaoaaa.xyz/tuchuang/images/2022/01/27/image-20220127230814325.png)

    

