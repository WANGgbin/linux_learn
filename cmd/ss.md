几个疑问：
- 有了 netstat 为什么还要使用 ss 

    ss 即 socket statistics. 其原理是读取`/proc/net/*`下各文件信息. 相比于 netstat，ss能够获取到更多的信息且似乎性能更好，在 netstat 的描述中可以看到：`This program is obsolete(过期)`，统计 socket 信息，建议使用 ss 
    > ss 实现参考[ss.c](https://github.com/shemminger/iproute2/blob/main/misc/ss.c)

常见操作：
- ss 的选项跟 netstat 的选项类似，常见的有:
- `-a` 所有套接字
- `-l` 监听套接字
- `-t` tcp
- `-u` udp
- `-x/--unix` unix
- `-n` numeric port not service name
- `-p` process


eg: 查看 unix 套接字状态 `ss -a --unix -p` 