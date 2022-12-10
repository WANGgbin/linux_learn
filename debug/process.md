## 如何查看进程下线程的数量
- cat /proc/id/status 
  可以看到有个 `Threads`字段,表示当前进程下线程的数量
- cat /proc/id/sched
  第一行可以看到进程的 name, id, 线程数量.
- 通过 `top -H -p pid`