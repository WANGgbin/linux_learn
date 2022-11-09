描述系统启动流程。
- init

    系统启动的第一个进程是`init`，init 根据配置文件`/etc/inittab`决定执行什么样的初始化任务。
- inittab

    先贴一下该文件的内容(以 debian 举例)
    ```sh
    # /etc/inittab: init(8) configuration.
    # $Id: inittab,v 1.91 2002/01/25 13:35:21 miquels Exp $

    # The default runlevel.
    # 2 代表系统的默认运行级别
    id:2:initdefault:

    # Boot-time system configuration/initialization script.
    # This is run first except when booting in emergency (-b) mode.
    # 启动时，系统的初始化脚本
    si::sysinit:/etc/init.d/rcS

    # What to do in single-user mode.
    ~~:S:wait:/sbin/sulogin

    # /etc/init.d executes the S and K scripts upon change
    # of runlevel.
    #
    # Runlevel 0 is halt.
    # Runlevel 1 is single-user.
    # Runlevels 2-5 are multi-user.
    # Runlevel 6 is reboot.

    # 一般有 7 种运行级别。一般我们使用3/5。根据前面获取的系统运行级别，只运行特定目录下的脚本。在 debian 下对应的目录为：/etc/rcx.d/
    l0:0:wait:/etc/init.d/rc 0
    l1:1:wait:/etc/init.d/rc 1
    l2:2:wait:/etc/init.d/rc 2  # 不包含网络功能的纯文本模式(多用户)
    l3:3:wait:/etc/init.d/rc 3  # 还有网络功能的纯文本模式(多用户)
    l4:4:wait:/etc/init.d/rc 4
    l5:5:wait:/etc/init.d/rc 5  # 窗口模式
    l6:6:wait:/etc/init.d/rc 6
    # Normally not reached, but fallthrough in case of emergency.
    z6:6:respawn:/sbin/sulogin

    # What to do when CTRL-ALT-DEL is pressed.
    # 按下 CTRL-ALT-DEL 重启系统
    ca:12345:ctrlaltdel:/sbin/shutdown -t1 -a -r now

    # Action on special keypress (ALT-UpArrow).
    #kb::kbrequest:/bin/echo "Keyboard Request--edit /etc/inittab to let this work."

    # What to do when the power fails/returns.
    pf::powerwait:/etc/init.d/powerfail start
    pn::powerfailnow:/etc/init.d/powerfail now
    po::powerokwait:/etc/init.d/powerfail stop

    # /sbin/getty invocations for the runlevels.
    #
    # The "id" field MUST be the same as the last
    # characters of the device (after "tty").
    #
    # Format:
    #  <id>:<runlevels>:<action>:<process>
    #
    # Note that on most Debian systems tty7 is used by the X Window System,
    # so if you want to add more getty's go ahead but skip tty7 if you run X.
    #
    # 系统启动默认打开 6 个终端，便是通过这里的设置完成的。当用户退出登录时，之所以重新到登录界面是因为 respawn 选项，导致 init 重新拉起 /sbin/getty 进程。
    1:2345:respawn:/sbin/getty 38400 tty1
    2:23:respawn:/sbin/getty 38400 tty2
    3:23:respawn:/sbin/getty 38400 tty3
    4:23:respawn:/sbin/getty 38400 tty4
    5:23:respawn:/sbin/getty 38400 tty5
    6:23:respawn:/sbin/getty 38400 tty6

    # Example how to put a getty on a serial line (for a terminal)
    #
    #T0:23:respawn:/sbin/getty -L ttyS0 9600 vt100
    #T1:23:respawn:/sbin/getty -L ttyS1 9600 vt100

    # Example how to put a getty on a modem line.
    #
    #T3:23:respawn:/sbin/mgetty -x0 -s 57600 ttyS3

    #-- daemontools-run begin
    SV:123456:respawn:/usr/bin/svscanboot
    #-- daemontools-run end
    ```
    文件说明：
    
    文件每一行的格式为：[设置选项名]:[run level]:[init 操作行为]:[命令选项]    
    - 设置选项名唯一的标识了一个选项。
    - run level: 系统的运行级别。表示当前设置选项在哪些运行级别下生效。
    - init 操作行为 可选的值有：
      - `initdefault` 系统默认的运行级别
      - `sysinit` 系统初始化执行的选项，比如前面文件中的：`si::sysinit:/etc/init.d/rcS` 表示/etc/init.d/rcS 来完成初始化。
      - `ctraltdel` 代表 ctrl + alt + del 是否可以重新启动的标志
      - `wait` 执行完当前选项，才可以执行后面的选项
      - `respawn` 后面字段代表的任务退出时，重新拉起任务。
    - 命令选项：执行什么样的命令以及命令参数
  
    > `/etc/inittab` 是按序执行的

- /etc/rcx.d

    init 根据 run level，来运行特定目录下的脚本。我们这里以 /etc/rc5.d 举例。首先查看下目录下的内容：
    ```sh
    lrwxrwxrwx   1 root root   15 May  7  2021 S01acpid -> ../init.d/acpid
    lrwxrwxrwx   1 root root   14 May  7  2021 S01atop -> ../init.d/atop
    lrwxrwxrwx   1 root root   18 May  7  2021 S01atopacct -> ../init.d/atopacct
    lrwxrwxrwx   1 root root   16 May  7  2021 S01auditd -> ../init.d/auditd
    ```
    我们会发现以下几个有意思的现象：
    - 所有文件都是链接到 `/etc/init.d/`目录下某个文件的软链接。因此 rcx.d 是 init.d 的一个子集。如果我们想添加一个系统启动任务，除了要在 /etc/init.d/ 下添加对应的脚本外，还需要在对应的 rcx.d 下创建软链接。
    - 我们知道 init.d 下的脚本可以启动、停止对应的任务。因此，在 rcx.d 中还必须通过某种方式来判断是启动还是停止任务。这便是通过文件名来区别的。**rcx.d 下的文件以 Sxx 或者 Kxx 开头，S 表示启动服务，K 表示停止服务， 至于后面的两位数字，则表示服务启动的顺序，因为某些服务之间是有依赖顺序的，因此被依赖服务应该优先启动**

- /etc/rc.local

    如果用户想自定义一些自启动任务，则可以通过 rc.local 脚本指定。

- systemd
    > 参考
    > [systemd 教程1](https://www.ruanyifeng.com/blog/2016/03/systemd-tutorial-commands.html)
    > [systemd 教程2](https://www.ruanyifeng.com/blog/2016/03/systemd-tutorial-part-two.html)


    几个疑问
    - 有了 init 为什么还要有 `systemd`
        - 因为 init 需要我们在 /etc/init.d 目录下实现自己的服务启动脚本，比较复杂。反观 `systemd` 只需要在对应的配置文件的[Service]设置具体的行为即可。
        - init 是串行执行服务的，所以性能较差。而 `systemd` 是并行启动服务的，性能较好。`systemd`中加入了一些特殊机制(比如预先创建套接字机制)，使得某些依赖的服务，也可以并行执行。具体可以参考：[浅析 Linux 初始化系统，第 3 部分: Systemd](https://mp.weixin.qq.com/s/mP-9CfgSwRycIBFaQ64Z2Q)

    - systemd
        systemd 能够并行启动服务的前提是**并行的服务之间没有依赖关系**，在 init 中，服务的依赖关系是通过 /etc/rcx.d 中文件名的数字决定的。在 systemd 中，服务之间的先后顺序以及依赖关系是通过配置文件中的[Unit]分节设置的。

    - 常见操作：
      - 只有 enable 的 unit 才有可能在系统启动的时候，开启对应的服务. 使用命令 `systemctl enable unit` 即可在`/etc/systemd/system`下创建对应的符号链接。
      - 启动服务 `systemctl start xxx[.service]`，有没有 service 后缀都可以
      - 停止服务 `systemctl stop xxx[.service]`
      - 杀掉服务 `systemctl kill xxx[.service]`
      - 重启服务 `systemctl restart xxx[.service]`
      - 查看服务状态 `systemctl status xxx[.service]`
      - 读取服务配置文件 `systemctl cat xxx[.service]`
      - 查看依赖关系 `systemctl list-dependencies xxx[.service]`
      - 重启机器 `systemctl reboot`
      - 关机 `systemctl poweroff`
    
    - unit
      - service 表示系统服务
      - target 多个 unit 构成一个组，target 类似于 init 中的 run level
      - socket 进程间通信 socket(pipe or unix socket)
        > 配置 xxx.socket，与此 socket 对应的服务默认为 xxx.service. 在 xxx.service 内部可以通过函数`sd_listen_fds`获取 socket 对应的描述符。细节可以参考: [sd_listen_fds](https://www.freedesktop.org/software/systemd/man/sd_listen_fds.html)

    - 配置文件
        
        不同的 Unit 可能会有自己专属的分区，比如socket 有 [socket]分区，service 有 [service] 分区，我们这里主要描述 [service] 的书写规则。
        > 配置文件大小写敏感
        - `Unit` 描述依赖关系
        - `Install`
          - `WantedBy` 描述由哪个 target 依赖。其实与 `Unit` 中的want 类似，描述依赖关系。当执行`systemctl enable`的时候，该服务配置对应的链接文件，会放到 target+.wants 目录中。
        - `Service` 描述如何操作服务
            - `Type`
              - `simple` 默认值，执行 ExecStart 指定命令，启动主进程
              - `forking` ExecStart 指定的命令以 fork 形式启动进程，同时父进程退出，这种情况下， systemd 会等待 ExecStart 指定的命令退出。
              - `oneshot`  systemd 会等待服务退出
              - `notify` ExecStart 指定的服务应该发送 notify 信号给 systemd。systemd 收到此信号后，才继续执行


    > **目前很多新版的 unix-like 系统都已经使用 systemd 替换了 init，因此建议使用 systemd 添加系统自启动服务。**

- journalctl

    守护进程`/lib/systemd/systemd-journald`用来对由 systemd 启动的服务的日志进行监听并处理。那么`journald` 的工作原理是什么呢？实际上由 systemd 启动服务的时候，会创建`Unix 套接字`然后将服务进程的 `standard output` 和 `standard error` 重定向到unix 套接字。`journald` 负责监听这些套接字，并将输入进行格式上的转化并记录到日志中。
    > 可以在 /proc/pid/fd/ 查看文件描述符的指向。对于 Unix 套接字，可以通过`ss -a --unix -p` 查看链接的具体信息.

    `journald`对应的配置为`/etc/systemd/journald.conf`，具体的配置项的含义，可以参考：`man journald.conf`

    我们列举几个重要的配置：
    - `Storage=`
        
        该选项控制应该在什么地方存储日志数据。有以下的可选值，默认值为`auto`
        - `volatile`

            数据只存储在内存中，通常是在 `/run/log/journal`，目录不存在的话会被创建。
        - `persistent`

            数据会存储到磁盘中，通常是在`/var/log/journal`，目录不存在的话会被创建。
        - `auto`

            与 persistent 类似，但是如果`/var/log/journal`不存在的时候，并不会主动创建，因此``的存在与否控制了数据是存储到内存还是磁盘中。 `auto + 手动创建 /var/log/journal == persistent`

        - `none`

            丢弃接收到的日志，但是转发到其他的目标的机制仍然正常工作(比如：转发到控制台或者是 syslog socket)。
    - 其他配置，按需添加

    > journalctl 的工作原理就是读取 journald 生产的日志文件，因此如果`Storage=none`，则 journalctl 无法正常工作。


    journalctl 常见的使用方法：
    > 通过 `man journalctl` 查看 journalctl 的详细说明
    - 查看所有日志 `journalctl`，**后续以 this 只带 journalctl**
    - 查看内核日志 `this -k`
    - 查看本次启动的日志 `this -b [-0]`
    - 查看上次启动的日志 `this -b -1`
    - 实时滚动展示最新日志 `this -f`
    - 展示最新的几行日志 `this -n number`
    - 反转输出，先展示最新的日志 `this -r`
    - 控制输出格式 `this -o`
      - `short` 默认，输出格式类似于 syslog
      - `json-pretty` 可读性较好的多行 json 格式
    - 展示特定 unit 的日志 `this -u xxx`
    - 展示特定级别的日志 `this -p **`
      - emmrg 0
      - alert 1
      - crit 2
      - err 3
      - warning 4
      - notice 5
      - info 6
      - debug 7
      
      可以通过数字或者字符串指定日志级别。如果只指定一个值，小于等于该级别的所有日志都会展现。也可以通过`FROM..TO.`指定一个范围，比如`warning..debug`表示查询级别在 warning 到 debug 之间的日志。当然，如果只展示某个级别的话，可以通过指定FROM和TO相等即可，比如只展示debug日志，`debug..debug`

    - 指定时间范围内的日志 `--since==` 某个时间点开始，`--until=`截止某个时间点的数据。数据的格式为`2022-10-09 15:26:00`，也可以指定为`yesterday`、`today`、`tomorrow`、`now`。
    - 可以通过指定脚本或者可执行文件的绝对路径来查看特定服务的日志

- 如何添加自启动任务
  - 先在目录 init.d 下添加对应的脚本，注意脚本要支持 start、stop、restart 等几个选项，然后在对应的 rcx.d 下创建对应的软链接文件
  - 在 rc.local(如果没有，就创建)里，指定对应的任务
  - 通过 systemctl
    - 首先，在`/lib/systemd/system` 下创建对应的配置文件 `my_first.service`，注意[Install] 中需要指定 `WantedBy=multi-user.target`，否则不生效。同时需要注意[Service] 中的进程Type，可以参考[systemctl 实现开机自启服务](https://blog.csdn.net/qq_29663071/article/details/80814081)
    - 使用 `systemctl enable my_first.servie`，则在 `/etc/systemd/system/multi-user.target.wants`下创建到`/lib/systemd/system/my_first.service`的链接文件
    - 重启系统即可
    > 当然，我们也可以自定义自己的 target. 然后此 target 加入到 multi-user.target 的 want。然后可以在此自定义 target 的 want 中加入自定义的服务即可。
    比如，定义 my_service.target，内容如下所示：
    ```text
    [Unit]
    Description=services added by us
    [Install]
    WantedBy=multi-user.target
    ```
    然后自定义 my_frist.service，内容如下所示：
    ```text
    [Unit]
    Description=my first service
    [Service]
    Type=simple
    ExecStart=/home/wangguobin.ultra/test/a.out
    Restart=on-failure
    [Install]
    WantedBy=my_service.target
    ```

- 如何查看当前运行级别
  `runlevel` 可以查看前一个以及当前的运行级别

- 如何更改系统的运行级别
  - 如果是永久更改的话，更改 inittab 对应的设置即可，然后重启服务
  - 如果是临时更改，则可以使用命令`init x`。该命令工作原理为比对 新的 rcx.d 文件相对于旧的 rcx.d 目录新增的文件，然后启动/关闭对应的任务。