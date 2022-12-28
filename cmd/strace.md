strace 命令可以用来查看某个进程的系统调用情况. 有两种方式:
- 以 strace 来执行程序

    trace ./exe
- 将 trace attach 到某个 pid 上

    trace -p pid

关于 strace 的使用可以参考:[十个例子让你了解 strace 的使用技巧](https://mp.weixin.qq.com/s?__biz=MzU4MzU4NzI5OA==&mid=2247486068&idx=1&sn=3e4797d776a6231594076e640d7c04df&chksm=fda78ef9cad007ef8d34fbaa64f49541956bf7eb86a0a001f9cd1f88623fa1a76f55c6b765e5&mpshare=1&scene=1&srcid=12253K4h8q16jJAOpiOTRsFy&sharer_sharetime=1671938894659&sharer_shareid=2066a9c7fd20aa6254f190f4435b83f8#rd)
