futex 是 `fast userspace mutual exclusion` 的简称.是一种用户态结合内核态的同步机制. c 中的 `pthread_mutex_t`底层就是基于 futex 实现的.

几个问题:
- 为什么要有 futex
  futex 之前的线程同步机制,占锁或者释放锁的时候都需要调用系统调用陷入内核. 然而,事实上,多个线程同时抢占一个锁的情况是比较少见的,大多数时候,某一时刻,只有一个线程会访问锁,因此,没有必要每次占锁或者释放锁的时候都陷入内核. futex 正是基于这样一个前提,首先尝试在用户态占有锁,如果占有成功的话,直接返回没有必要陷入内核.释放锁也是同样的思路,释放锁之后,如果没有其他线程在等待这把锁,也没有必要陷入内核去 wake 其他线程.

- 内核实现
  在 linux 内核中, futex 是通过系统调用的方式对用户提供服务的. 参考 `kernel/futex.c`.
  futex 系统调用的原型为:
  ```c
    /*
    uaddr1: 通常为锁的地址
    op: 操作类型.有两类操作: FUTEX_WAIT, FUTEX_WAKE
    val1: uaddr1 之前的值
    utime: 阻塞的时间
    uaddr2,val2 基本不使用
    */
    int futex(u32 *uaddr1, int op, u32 val1, struct _kernel_timespec * utime, u32 *uaddr2, u32 val2);
  ```
  当 op == FUTEX_WAIT 的时候,内核中调用的是 `futex_wait()`, 当 op == FUTEX_WAKE 的时候,内核调用的是 `futex_wake()`.

  内核中, futex 是基于 hashtable 实现的. key 就是 *uaddr1. 我们简单描述下两个函数的逻辑.
  - futex_wait

    1. 再次获取 uaddr1 上的值. 

    从用户空间到调用 futex 之间有一个时间窗口, 该时间窗口内锁的值可能会发生变化.因此,需要判断锁的值有没有发生变化,如果发生变化,则直接返回.用户空间再次尝试占有锁.否则,可能导致线程永久阻塞.
    2. if(newval == val1)
       1. newval != val1, 函数返回
       2. newval == val1,进入下一步
    3. 占有哈系表对应的 bucket的自旋锁,并插入当前 task_struct,并调用 schedule().

  - futex_wake
    1. 找到 hashtable 对应的 bucket 并占有自旋锁
    2. 激活所有阻塞在 uaddr1 上的线程.
   > 实际上,如果当前没有阻塞在 uaddr1 上的线程的话, wake 线程完全没有必要抢占自旋锁,直接返回即可.进而提高性能. 怎么判断是否有阻塞的线程呢?实际上在 bucket 中有一个 waiters 的变量,表示当前 bucket chain 中元素的个数.

- 伪代码实现
  - 版本一
    - futex_wake
    ```c
    bu = find_bucket();
    spin_lock(bu.lock);
    wake_up();
    spin_unlock(bo.lock);
    ```
    - futex_wait
    ```c
    spin_lock(bu.lock);
    // 从陷入内核到占有锁有个时间窗口, addr1 的值可能已经发生变化,需要再次判断.
    newval = *addr1;
    if(newval != val1) {
        spin_unlock(bu.lock);
        return;
    }
    add_task();
    spin_unlock(bu.lock);
    ```
    此版本没有什么问题,但是可以进一步优化 futex_wake 函数. futex_wake 在陷入内核后, futex 的值可能已经发生变化了.因此如果没有线程阻塞的话,是没有必要占用自旋锁的,直接返回即可.
  - 版本二
    - futex_wake
    ```c
    bu = find_bucket();
    if (bu.waiters == 0) { // 如果 waiter == 0, 直接返回.
        return;
    }
    // 同版本一
    ...
    ```
    - futex_wait

    ```c
    spin_lock(bu.lock);
    if(newval != val1) {
        spin_unlock(bu.lock);
        return;
    }
    bu.waiters++; // 增加 waiter 
    add_task();
    apin_unlock(bu.lock);
    ```
    此版本加入了 waiter. 看着似乎没有问题.实际上是有问题的. 因为 futex_wait 的 waiters == 0 判断逻辑并没有通过自旋锁保证串行化,因此可以发生在 futex_wait 执行的任何阶段.考虑一下执行顺序:
    wait:                           wake:
                                    bu = find_bucket();
                                    if(bu.waiters == 0) {return;}
    bu.waiters++;
    add_task();

    这会导致 wait 线程永远阻塞. 通过调整 wait 中 bu.waiters++ 的位置,可以解决此问题.
  - 版本三:
    - futex_wake
        同版本二
    - futex_wait
    ```c
    spin_lock(bu.lock);
    bu.waiters++;
    if(newval != val1) {
        bu.waiters--;
        spin_unlock(bu.lock);
        return;
    }
    // bu.waiters++; // 增加 waiter 
    add_task();
    apin_unlock(bu.lock);
    ```
    此版本看着似乎没有问题了.实际上,还是有问题的.这里有个特别有意思的地方就是会涉及 `store-load` 乱序问题.
    现在两个函数的逻辑,可以简化为:
    futex_wait:
    newval = *addr1; // load addr1
    spin_lock();
    bu.waiters++;  // store waiters
    ...

    futex_wake:
    *addr1 = newval; // store addr1
    if(bu.waiters == 0) { // load waiters
        ...
    }
    ...

    之所以会发生乱序是因为**多个线程同时访问 addr1 和 waiters 变量,且没有通过其他同步机制来保证串行化**. 因此就可能导致, wait 修改了 waiters 但是 wake 看不到, wake 修改了 addr1,但是 wait 看不到的情况. 这就会导致 wait 线程永久阻塞. 该问题属于一个经典的模型.
    ```c
    x = y = 0;

    routine1:                   routine2:
    w[x] = 1                    w[y]=1
    r[y]                        r[x]
    ```
    没有加屏障的话,上面就可能会出现 r[y] = 0 && r[x] == 0 的情况.
    要解决此问题,我们需要加入屏障保证修改数据后的全局可见性(其他 cpu 可以立即看到变量的更改.).
    ```c
    x = y = 0;

    routine1:                   routine2:
    w[x] = 1                    w[y]=1
    mb()                        mb()
    r[y]                        r[x]
    ```
    屏障保证了,在其他 cpu 角度看来,语句执行顺序同代码中的顺序.
  - 版本四
    - futex_wake
    ```c
    *addr1 = newval;
    bu = find_bucket();
    mb(); // 加入 fence
    if (bu.waiters == 0) { // 如果 waiter == 0, 直接返回.
        return;
    }
    - futex_wait
    ```c
    spin_lock(bu.lock);
    bu.waiters++;
    mb(); // 加入 fence
    if(newval != val1) {
        bu.waiters--;
        spin_unlock(bu.lock);
        return;
    }
    // bu.waiters++; // 增加 waiter 
    add_task();
    apin_unlock(bu.lock);
    ```
- 总结
  futex 是现代线程同步实现的基础. 其本身的实现是一个`store-load乱序`的经典场景.