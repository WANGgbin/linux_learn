学习 linux 系统中的定时任务.
几个疑问:
- 提供给用户的接口是什么?用户如何创建定时任务?
- 定时任务的工作原理是什么

## crontab
crontab 是用来干什么的呢?这是提供给用户的接口,用户通过该接口可以创建,查看,删除定时任务.
命令选项:
- -e: 创建一个新的定时任务
- -l: 查看该用户所有的定时任务
- -r: 删除所有定时任务
> 如果想删除某一个个任务,可以通过 -e. crontab 实际上会在目录`/var/spool/cron/crontabs`下为每个用户创建了一个文件,文件内部存储了用户的所有定时任务.

1. 安全管理

为了安全考虑,并不是所有用户都可以创建定时任务.通过在配置文件`/etc/cron.deny`添加用户名,限制该用户创建定时任务.
```sh
wgb@asus-x555ld:~/tmp$ crontab -e
You (wgb) are not allowed to use this program (crontab)
```

## /etc/crontab
除了可以通过`crontab`创建定时任务外,还可以通过修改 `/etc/crontab`配置文件来添加定时任务. /etc/crontab 是系统范围的配置文件,我们看看配置文件的内容:
```text
SHELL=/bin/sh
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin

# Example of job definition:
# .---------------- minute (0 - 59)
# |  .------------- hour (0 - 23)
# |  |  .---------- day of month (1 - 31)
# |  |  |  .------- month (1 - 12) OR jan,feb,mar,apr ...
# |  |  |  |  .---- day of week (0 - 6) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
# |  |  |  |  |
# *  *  *  *  * user-name command to be executed
17 *	* * *	root    cd / && run-parts --report /etc/cron.hourly
25 6	* * *	root	test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.daily )
47 6	* * 7	root	test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.weekly )
52 6	1 * *	root	test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.monthly )
```
可以看到与`/var/spool/cron/crontabs`目录下文件内容不同的地方在于,多了`user-name`一项./var/spool/cron/crontabs 是通过文件名来区分用户的.
另一个需要关注的是几个特殊的目录:
- `/etc/cron.hourly`
- `/etc/cron.daily`
- `/etc/cron.weekly`
- `/etc/cron.monthly`

这几个是目录,内部为定时任务.`run-parts`会遍历执行目录下所有的任务.所以,我们可以直接在对应的目录下添加文件,即可创建定时任务.当然,我们可以创建目录.
所以,如果我们想创建一个每1min 执行的定时任务,有以下几种创建方式.
> 假设待执行文件路径为: /job/to/run
- crontab -e 然后键入: `*/5 * * * * /job/to/run`
- 编辑 /etc/crontab 文件,键入: `*/5 * * * * wgb /job/to/run`
- 创建目录 /etc/jobs_per_5min, 编辑 /etc/crontab 文件: `*/5 * * * * wgb run-parts /etc/jobs_per_5min`

## 注意事项
1. 如果同时运行多个定时任务,可能会导致当前系统负载过重. 所以需要仔细的分配一下运行时间.这里借鉴`鸟哥Linux私房菜`中的例子说明如何分配时间.
假设有4个每5min执行的任务,则可以这样划分任务执行时间:
```sh
vi /etc/crontab
1,6,11,16,...,56 * * * * root CMD1
2,7,12,17,...,57 * * * * root CMD2
3,8,13,18,...,58 * * * * root CMD3
4,9,14,19,...,59 * * * * root CMD4
```

2. 安全检验
很多时候植入的木马都是通过定时任务的方式植入的,所以可以检查`/var/log/syslog`内容,查看是否运行了意料之外的定时任务.

3. 时间格式
周不能与日,月 同时指定,有歧义.

## anacron
如果机器某个时间段关机,那么本应在该时间段运行的任务无法运行,那怎么办呢?这就是 anacron 存在的意义,即兜底未运行的任务.
首先看看配置文件`/etc/anacrontab`内容:
```sh
1	5	cron.daily	run-parts --report /etc/cron.daily
7	10	cron.weekly	run-parts --report /etc/cron.weekly
@monthly	15	cron.monthly	run-parts --report /etc/cron.monthly
```
> 该配置文件指定了 anacron 要执行哪些任务,anacron 可以执行的任务必须是在 /etc/anacrontab 中存在的. 我们可以在 /etc/anacrontab 中添加任务

anacron 通常是在开机时运行的,我们以第一行举例,来看看 anacron 是如何工作的.
首先会检查上一次执行 /etc/cron.daily 的时间(`/var/spool/anacron`下与 /etc/anacrontab 中同任务名的文件,保留上次任务执行的 date),假设为 last_run_time,然后判断 cur_time - last_run_time 是否大于 1 天(**第一列单位: day**),如果是,则在开机之后 5min(**第二列单位: min**), 执行任务 `run-parts --report /etc/cron.daily`, 执行完之后,更新/var/spool/anacron 下对应的文件,如果文件不存在则创建文件. 当/etc/anacrontab 里面所有的任务都执行完毕后, anacron 退出.

那么 anacron 是如何记录上次任务执行时间的呢? 我们查看 /etc/cron.daily 目录,发现地一个文件为`0anacron`,其内容为:
```sh
text -x /usr/sbin/anacron || exit 0
anacron -u cron.daily  # 会在 /var/spool/anacron/下创建对应的文件,文件内容为当前 day
```