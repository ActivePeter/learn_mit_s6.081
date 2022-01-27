# learn_mit_s6.081

- 链接
  - https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/
  - https://pdos.csail.mit.edu/6.828/2020
  - [6.S081组队刷课总结 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/347108301)
  - [xv6: a simple, Unix-like teaching operating system (mit.edu)](https://pdos.csail.mit.edu/6.828/2020/xv6/book-riscv-rev1.pdf)
- 注意点
  - 虚拟机里不要把代码放在共享硬盘中，会导致编译有玄学问题
  - lib1中很多系统调用，
    - 得了解清楚系统调用的用法（xv6的文档属实没写请
    - 其中第三个卡了很久，最后发现pipe的读写关闭需要在两个线程中关闭，建议各位在遇到问题时直接去查询linux对应得系统调用用法
- [lib1](./lib1.md)

