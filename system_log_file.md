描述 linux 系统中的比较关键的日志文件以及关于日志的 daemon.

几个疑问：
- 不同 unix-like 系统，日志文件路径是不完全相同的，对于一个具体的系统，如何知道都有哪些登录文件呢？
    一般日志文件都在`/var/log`下，具体的内容，可以结合 rsyslogd 配置文件 `/etc/rsyslog.conf` 查看 rsyslogd 将哪些内容存到哪些文件中。对于其他的文件，则可以百度 - ^ _ ^ - 

- 日志文件仅由 system-journald 和 rsyslogd 这两个 dameon 维护吗？是否存在不由这两个 daemon 管理的日志文件？如果有的话，是哪些？为什么不交由 rsyslogd 维护？
    当然还有其他的三方服务，完全可以自定义日志文件的路径，不过通常也定义在 `/var/log`下，方便管理。

    大多数日志都在目录`/var/log/`下。而此目录下，大多数文件由 rsyslogd 生成。具体哪些内容生成到哪些文件中，由配置`/etc/rsyslog.conf`决定。除了 rsyslogd 生成的文件外，还有几个比较重要的文件。
    - `/var/log/wtmp` 和 `/var/log/btmp`
  
        其中，wtmp 记录正确登录系统的账号信息, faillog 记录错误登录系统的账号信息。如何读取这两个文件的内容呢。通过 `last` 和 `lastb` 命令，前者读取 wtmp 内容，后者读取 btmp 内容。
    - `/var/log/lastlog`

        记录系统上面所有账号最近一次的登录信息。可以使用`lastlog`查看此文件内容。

    - `/var/log/xxx/`

        不同的网络服务可能会有自己专属的日志文件，比如 httpd等。

- 对于系统日志文件，有没有工具可以帮助我们查看具体内容呢？还是只能手写 shell?
    可以基于 `journalctl` 命令 查看并分析日志文件。
- 日志服务似乎已经有日志轮替的功能了，那什么时候需要使用 logrotate 轮替日志呢？

    linux 系统本身就是通过 logrotate 程序完成日志的轮替的。logrotate 被加入到 `/etc/cron.daily`目录中，是一个天级的定时任务。logrotate 通过配置文件 `/etc/logrorate.conf` 控制什么样的日志，如何轮替。

    我们看看 `/etc/logrotate.conf`的内容：
    ```text
    # 多久 rotate 一次
    weekly 

    # 总共多少个备份
    rotate 4

    # create new (empty) log files after rotating old ones
    create

    # 是否压缩日志文件
    #compress

    # packages drop log rotation information into this directory
    include /etc/logrotate.d

    # 针对于某个特定日志的设置，覆盖上述的默认设置
    /var/log/wtmp {
        missingok
        monthly
        create 0664 root utmp
        rotate 1
    }

    /var/log/btmp {
        missingok
        monthly
        create 0660 root utmp
        rotate 1
    }
    ```

    **我们可以在 /etc/logrotate.conf 中，设置任意一个日志文件的 rotate 规则**。这里有一点特别值得我们学习，就是`include /etc/logrotate.d`。对于配置文件，可以有两种方式，一种是全部的内容，只写到一个文件中，另一种就是通过 `include` 的方式。`include`方式的优点在于符合`开闭原则`，当我们需要扩展新的配置的时候，无需修改`/etc/logrotate.conf` 只需要在 `/etc/logrotate.d` 中加入我们自己的配置文件即可。

    如果想轮替某个日志，就在`/etc/logrotate.d`加入对应的配置文件即可。

- rsyslogd 的工作原理？
    
    该服务的定位是负责记录系统范围内的日志。其工作原理是在 Unix-socket 或者 tcp/udp socket 上监听其他服务发送的日志消息。然后根据配置文件`/etc/rsyslog.conf`将不同 facility 和 level 的信息存储到不同的日志中。
    
    其他服务如果想通过 rsyslogd 记录日志信息，就需要通过`syslog`函数，建立与 rsyslogd 的套接字连接，同时告诉 syslog，自己发送的消息的 facility 和 level 是什么， syslog的具体使用可以参考`man syslog`。

    我们这里注意下 rysslog.conf 的语法，具体可以参考：[鸟哥的 Linux 私房菜: 基础学习篇](https://tiramisutes.github.io/images/PDF/vbird-linux-basic-4e.pdf)

    支持 `system-journald`的系统中， journal 在默认情况下会将收集到的信息都转发到 rsyslogd，可以通过 `/etc/systemd/journald.conf`中的 `#ForwardToSyslog=yes`更改此设置(建议使用默认设置)。

- 如何发消息到 rsyslogd
    两种方式：
    - 应用内部可以调用 `syslog`相关函数，发送信息到 rsyslogd。
    - 也可以通过`logger`命令，发送信息到 rsyslogd， logger 最常用的使用方法是 `logger -p facility.level message`
  
- 如何通过 rsyslogd 将本地日志发送到某个服务器？
    - 服务端

        修改`/etc/rsyslog.conf`，根据 udp 或者 tcp 选择取消注释，这样 rsyslogd，就会从 port 514 上监听数据。
        ```text
        # Provides UDP syslog reception
        #$ModLoad imudp
        #$UDPServerRun 514

        # Provides TCP syslog reception
        #$ModLoad imtcp
        #$InputTCPServerRun 514
        ```
    - 客户端

        客户端只需要指定服务端的 ip:port(默认是 514) 即可，对于 tcp: `*.* @@remote-host:514`， 对于 udp: `*.* @remote-host:514`，udp 比 tcp 少了一个 `@`

    



