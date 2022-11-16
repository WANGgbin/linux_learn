描述 linux 中的守护进程。
问题：
- 什么是终端？
    早期的计算机只有一个控制台，为了支持多用户同时作业，通过将电子打印机(teletype)连接到主机上，进行输入输出。这里的电子打印机就是最早的终端。所以现在的终端简称为 `tty`。终端就是设备，在 linux 中，凡是能够进行输入/输出的都可以看作是一个文件，终端设备也不例外，在 `/dev/`目录下存放与终端设备对应的设备文件。
- shell 与终端关系？
    shell 进程的 0、1、2是指向终端设备文件的，负责从终端设备读取用户命令，执行，然后将输出发送到终端设备。
- 用户登陆的时候发生了什么？
    可以参考 apue 第九章。大概流程是：
    - 系统启动的时候，会针对每个可用的终端设备都会 fork 一个进程。
    - 进程内 exec `getty`程序，该程序主要负责打开设备文件，然后将 0、1、2 重定向到终端。接着在终端输出`login:`字样，提醒用户输入用户名。
    - 用户输入用户名后，`getty`会将用户名传递给`login`程序。
    - `login`程序，在终端输出`passwd:`字样，提醒用户键入密码。然后进行用户校验。校验通过后，会切换到用户主目录，exec 用户对应的 shell 等一系列用户个性化操作。
    - 最后，进入到登陆 shell，shell 等待从终端读取命令并执行。
- 怎么理解会话、作业、进程组？
    会话其实好理解，用户打开一个终端，进入一个会话。会话由若干进程组组成，包括一个前台进程组和若干后台进程组，一个会话**可以**关联一个终端设备(后面会讲到，守护进程是不关联终端的)。作业跟进程组可以视为相同的概念，通常支持作业的 shell 中，一个作业是由管道串起来的若干进程。比如：
        ps -efj | grep 'xxx'
        ps 跟 grep 构成一个作业。
    
    前台作业跟后台作业什么区别呢？
    前台作业占据终端，可以直接与终端进行交互。如果我们想让作业在后台运行，然后在终端创建其他作业，就可以让作业在后台运行。对于新建的作业，通过在命令末尾添加`&`指定该作业为后台作业。对于运行中的作业，可以先通过`ctrl + z`挂起当前作业，然后通过`bg`命令，切换作业到后台执行。
    比如：
    ```shell
    (base) wangguobin.ultra@n248-161-006:~/test$ ./a.out 
    ^Z
    [1]+  Stopped                 ./a.out
    (base) wangguobin.ultra@n248-161-006:~/test$ jobs
    [1]+  Stopped                 ./a.out
    (base) wangguobin.ultra@n248-161-006:~/test$ bg %1
    [1]+ ./a.out &
    (base) wangguobin.ultra@n248-161-006:~/test$ jobs
    [1]+  Running                 ./a.out &
    (base) wangguobin.ultra@n248-161-006:~/test$ 
    ```
    > 作业相关的命令：
    > - jobs: 查看当前回话所有的作业
    > - fg: 切换某个作业为前台作业
    > - bg: 切花某个作业为后台作业

    那么后台作业如果读/写终端会发生什么呢？
    如果读终端，则终端驱动程序会发送`SIGTTIN`信号给后台进程，导致当前进程停止运行。如果是写终端，默认是可以直接写终端，通过命令`stty tostop`，可以更改默认行为为发送`SIGTTOU`信号给后台进程，从而停止当前进程的执行。
- SIGHUP 是什么？
    当终端退出的时候，当前会话的所有进程都会收到 SIGHUP 信号。apue 描述：终端退出时，会发送信号给会话首进程(shell 进程)，会话首进程会把 SIGHUP 信号发送给前台进程组中的所有进程。但是，本人通过在`debian`和`ubuntu`测试后发现，后台进程组的进程同样会收到 SIGHUP 信号，奇怪的时候，后台进程组进程收到该信号并不会退出，与前台进程组中的进程行为相反。
    通常，我们会给守护进程发送 SIGHUP 信号，通知其进行某种操作，比如重新加载配置文件等。为什么会选择 SIGHUP 呢？因为守护进程没有相关的终端，所以不会收到 SIGHUP 信号，所以使用 SIGHUP 信号不会跟其他的场景冲突，没有歧义，很安全。
    > 可以通过 `nohup cmd` 的方式执行命令，该命令会忽略 SIGHUP 信号。

- 守护进程是什么？与普通进程的区别是什么？怎么创建守护进程？守护进程如何进行输入/输出？
  如果我们想让某个进展一直运行，则可以把该进程变为守护进程。那么如何将普通进程修改为守护进程呢？最关键的是，当前进程需要通过`setsid`新建一个会话(**通过 fork，确保当前进程不是某个进程组的组长进程，组长进程无法通过 setsid 创建会话**)，新建会话后：
    - 当前进程成为会话首进程
    - 当前进程成为进程组组长进程
    - 没有终端与当前会话关联

    同时，我们还需要关闭 0、1、2 与旧终端的关联，简单的关闭或者重定向到 `/dev/null`。这样，当前进程就不会跟任何的终端关联，就可以在后台一直运行！linux 中有很多的任务都是通过后台进程的方式运行的，比如：sshd、inetd、crond等

- 如何设置服务开机启动？服务开机启动流程是什么样的？
    
    首先**记忆**几个常用的工作目录。
    1. `/etc/init.d/` 系统启动脚本放置处。有两种方式可以运行这些脚本。第一种是以绝对路径方式直接运行，第二种是通过脚本`service`来运行。`service`常见的使用方式为：
       1. `service script command`，比如重启 crond 服务：`service crond restart`
       2. `service --status-all` 用来查看所有系统服务的状态。`[+]` 表示正在运行，`[-]`表示停止，`[?]`表示脚本不支持 `status`
    2. `/var/run/*` 各服务程序的 PID 记录处
    3. `/etc/rc.d/`
    4. `/etc/inittab`

- 为什么都是通过 shell script 来启动服务？
    1. 通常启动服务需要初始化工作环境，比如工作目录、环境变量等。使用 `sh` 就很方便初始化进程工作环境(原理：在 `fork` 和 `exec` 中，子进程很多属性都是直接继承父进程的)。
    2. 方便管理。sh 不仅可以用来启动服务，还可以用来关闭服务、重启服务、发送信号给服务等很多丰富的功能。

- 超级服务器 `xinted` 是如何工作的？
- 