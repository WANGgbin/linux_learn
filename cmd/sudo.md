问题：
- su 与 sudo 的区别是什么？ 什么时候使用 su 什么时候使用 sudo ？

su(substitite user)，用来切换用户。sudo 是以某个用户的身份执行某个命令。 关键的是， su 需要输入目标用户的密码，而 sudo 只需要输入自己的密码即可，在多用户系统上， sudo 更加安全。
- su 与 su - 区别是什么？

su 和 su - 均是切换到 root 账号的意思。但是相比于 `su`,  `su -` 切换到 root 更加彻底，比如像 PATH 环境变量。如果是 `su`, PATH 还是之前用户的 PATH， 而 `su -` 会设置为 root 对应的环境变量。
类似于 `su -`，如果要切换到别的用户，建议使用 `su -l user`. `-` 实际上就是 login-shell 与 non-login-shell 的区别。
此外，我们也可以使用 su 切换到某个账户并只执行一条指令然后返回到当前账号。方法就是通过参数 -c "command"。比如要查看 `/etc/shadow` 前5行，则可以：
```sh
    su - -c "head -n 5 /etc/shadow"
```
- 什么样的用户才可以 sudo 呢？

sudo 表示以什么样的身份执行什么样的命令。当然并不是所有的用户都可以执行 sudo 命令。可以通过修改 `/etc/sudoers` 指定哪些用户可以以哪些账号的身份执行哪些命令。因为 `/etc/sudoers`有一定的语法，因此不建议直接修改，而是使用专用的编辑命令 `visudo` 来编辑此文件。`/etc/sudoers` 语法格式为：
```sh
user ALL=(ALL) ALL # 用户名 登陆主机名=(可切换身份) 可执行命令
```
同样还可以指定某个用户组内的所有用户：
```sh
%wheel ALL=(ALL) ALL # 凡是属于用户组 wheel 的用户 都可以以任何身份执行任何命令
```
当然，以任何身份执行任何命令特权还是太高了，如果只限定以 root 身份更改账号密码呢？则可以执行：
```sh
user ALL=(root) /usr/bin/passwd # 命令路径必须是绝对路径
```
那如果想限制不能执行哪些命令呢？
```sh
user ALL=(root) !/usr/bin/passwd, /usr/bin/passwd [A-Za-z]*, !/usr/bin/passwd root # 命令前面加上 ! 表示不能执行这条命令，此外命令还可以使用正则表达式。
```
此外，sudo 还支持用户别名，命令别名。这有什么用呢？如果想给很多的用户配置相同的 sudo 权限，则可以使用此特性。
```sh
User_Alias SOMEBODY = user1, user2, user3
Cmnd_Alias SOMECMDS = !/user/bin/passwd, /usr/bin/passwd [A-Za-z]* # User_Alias, Cmnd_Alias 是关键字！此外，后面的 alias_name 必须是大写

SOMEBODY ALL=(root) SOMECMDS # SOMEBODY 指定的所有账号都可以执行 SOMECMDS 指定的命令。
```
- 有没有办法通过 sudo 长期切换到 root 账户？

可以通过 `sudo su -` 长期切换到 root 账号，则只需要输入自己的密码即可。