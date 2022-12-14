描述进程相关内容

几个疑问
- 作业相关

- 如何让某个进程在后台运行
    
    通过`nohup cmd &` 可以让某个进程在后台运行,即使相关的终端退出,当前进程也不受影响,原理是 nohup 会使 cmd 忽略 SIGHUP 信号.
- 如何查看系统当前各个进程运行情况
    `ps` 可以查看某个时间点,系统的进程信息. ps 的选项比较丰富,常见有三种使用方法.
    - `ps -l`查看当前 bash 相关进程
    - `ps [-]aux`查看系统当前所有进程
    - `ps -lA` 以 -l 格式查看系统当前所有进程

    如何动态查看系统进程信息呢?可以通过 `top`
    > 进入 top 页面后,我们是可以继续编辑的.

    top 输出两部分信息,第一部分是系统相关的,包括 cpu, mem, tasks 等. 第二部分是进程信息.
    top常见的选项有:

    - `-d seconds` 指定 top 界面更新频率
    - `-p pid_num` 查看某个具体的进程信息

    top 界面编辑命令:

    - `?` or `h` 查看可使用命令
    - `P` 以 cpu 利用率排序进程
    - `M` 以 mem 使用率排序进程
    - `N` 以 进程号排序进程
    - `T` 以 进程运行时间排序进程
    - `r` 更改某个进程的 nice(调度优先级) 参数
    - `q` 退出 top 界面


- 如何关闭某个进程

    通过 `kill` 或者 `killall` 可以关闭某个进程.

    使用 kill 需要注意的地方:
    - `kill -l` 可以查看支持的信号
    - 既可以作用于某个进程,也可以作用于某个作业 `kill -9 pid_num` or `kill -9 %job_num`

    另外,我们还可以通过进程名来删除进程, 这就是 `killall` 的作用. 常见的选项:
    - `-i` interactive,删除进程时,会接受用户的确认
    - `-I` 忽略大小写
- /proc 作用

proc 是特殊的文件系统, 非磁盘文件系统. 其存在的意义是通过提供与文件读写一致的接口,使得用户可以查看当前系统各个进程的运行情况.常见的进程信息有:
- `environ` 环境变量
- `exe` 对应的可执行文件的路径
- `cwd` 当前工作目录
## pstree
该命令用来查看进程树,有什么用呢?我们经常遇到的情况是**某个进程经常重启,除了可能由定时任务引起,另一种可能的情况就是由父进程不断拉起子进程**那么该如何查看该进程的父进程呢?一种方便的方法就是通过`pstree`.

常见的选项:

- `-p` 显示进程的进程号
- `-u` 显示进程所属的用户

## nice renice
新执行的命令,可以通过 nice 调整进程的调度优先级. `nice -n 数字 cmd`
> nice 调整范围: -20 - 19
> root 可调整的nice 范围: -20 - 19
> 普通用户可调整的nice 范围: 0 - 19,这意味着普通用户只能降低某个进程的优先级(pri = pri + nice, pri 越低,进程优先级越高)

renice 可以调整已经运行的进程的优先级`renice 数字 pid`

## 系统资源查看
- 查看内存使用情况 `free`, 默认单位是 KB, 可以通过`-b|-k|-m|-g`修改单位
- 查看系统相关信息 `uname -a`
- 查看网络情况 `netstat -[atulnp]`
- 查看系统资源(CPU/内存/磁盘输入输出/中断)情况: `vmstat`, 具体字段可以参考`鸟叔 Linux 基础 Char 17.`

