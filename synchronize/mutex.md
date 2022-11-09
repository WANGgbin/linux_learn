## 自旋锁
几个疑问。
- 基于什么实现的呢？
  基于原子操作实现，相当简单！
- 为什么占有自旋锁的时候，禁止内核抢占呢？
    因为占有自旋锁，通常很短时间内会释放锁，但是如果此时被抢占，则自旋锁占有时间太久，所以通常，由自旋锁保护的临界区是禁止内核抢占的。

## 互斥锁
关于互斥锁，几个疑问。
- 老是说锁性能差，为什么？
    关于线程锁，首先是用户态、内核态转化，存在上下文的切换。其次，在锁内部，还会导致线程阻塞，激活后又有调度延时。这些都导致使用线程锁的代价是比较大的。
- 琐是用来同步的，那么对于锁这数据结构的访问，又是通过什么同步的呢？
    我们查看 linux-5.9/kernel/locking/mutex.c 内核互斥锁的实现，可以看到 mutex 是一个结构体，定义为：
    ```c
    struct mutex {
        atomic_long_t		owner;
        spinlock_t		wait_lock;
        struct list_head	wait_list;
    }
    ```
    是通过**原子访问(最常见的:for(;;){} + cmpxchg)以及自旋锁**实现的。

- 优化的互斥锁会先自旋。那么怎么判断是否自旋以及自旋多久呢？
    linux 内核中，通过判断当前占有锁的线程是否正在运行，来判断是否自旋，如果占有锁线程还在运行，则任务很快会释放锁，所以自旋。否则，加入等待队列。
    golang 中的互斥锁也是优化过的锁，也有自旋的概念。 
- 引入自旋又会引入等待链表线程的饥饿问题，如何解决呢？
    handoff()机制，如果等待队列的第一个元素被激活，但是锁被偷了，那么会给锁的标记加上 `MUTEX_FLAG_HANDOFF`，当释放锁的时候，就会将等待队列第一个元素的 `task_struct`设置到 mutext.owner中，这样当队首元素被再次激活的时候，一定可以拿到互斥锁。

我们总结下 linux-5.9 中 mutex 的实现：
通过原子操作`cmpxchg(mutex.lock, 0UL, current)`尝试直接占有锁.
    -> 成功，返回
    -> 失败，进入 `slowpath`
        -> 进入一个死循环：
            -> 通过`__mutex_trylock`尝试占有锁
                -> 如果 task == null，占有成功，返回
                -> 如果 task != current, 占有失败，返回
                -> 如果 task == current && flag & MUTEX_FLAG_PICKUP(handoff机制，在释放锁的时候如果设置了MUTEX_FLAG_HANDOFF标志，则设置 MUTEX_FLAG_PICKUP)，则清理 MUTEX_FLAG_PICKUP、MUTEX_FLAG_HANDOFF标志，占锁成功返回
            -> 占有成功，返回。
            -> 获取占有锁的 task，判断 task 是否运行
            -> 是的话，当前线程自旋
            -> 否则，推出循环
        -> 进入一个死循环：
            -> 睡眠
            -> 重新运行，判断是不是第一个元素，如果是的话，设置标志 `MUTEX_FLAG_HANDOFF`
            -> 占锁成功，返回
            -> 进入下一次循环

## 读写锁
## rcu(读-拷贝-更新)
几个疑问：
- 何时释放旧的数据
- 新数据更新后，继续访问旧的数据是否会有问题
    看具体场景
- rcu 基于什么实现
    似乎很复杂。。。
rcu 最关键的思想是**将整个数据的同步访问转化为一个指针的访问，在更新数据的时候，只需要修改指针即可，修改指针本身是个原子性操作**，我们还可以在其他的场景中看到类似的思想，比如go语言中的 `atomic.Value`。至于 linux 内核中具体的实现，感觉很复杂，后面有机会再看。
可以参考：
- [Linux RCU](https://abcdxyzk.github.io/blog/2015/07/31/kernel-sched-rcu/)
- [Linux 锁机制](https://zhuanlan.zhihu.com/p/374902282)

## 顺序锁
优先保证写操作，作为代价，读操作可能需要重复读取。
```c
typedef struct {
	unsigned sequence;  // 写操作更改此值，读操作通过检测此值的变化，判断读到的值是否可信
	spinlock_t lock;  // 保证写操作之间互斥
} seqlock_t;
```
我们看看顺序锁给类操作的实现：
```c
/* Lock out other writers and update the count.
 * Acts like a normal spin_lock/unlock.
 * Don't need preempt_disable() because that is in the spin_lock already.
 */
static inline void write_seqlock(seqlock_t *sl)
{
	spin_lock(&sl->lock);
	++sl->sequence;
	smp_wmb();  // 注意写屏障的应用，保证更新 sequence 后，再执行具体的操作
}	

static inline void write_sequnlock(seqlock_t *sl) 
{
	smp_wmb();  // 保证执行完具体的操作之后，再更新 sequenence
	sl->sequence++;
	spin_unlock(&sl->lock);
}

/* Start of read calculation -- fetch last complete writer token */
static inline unsigned read_seqbegin(const seqlock_t *sl)
{
	unsigned ret = sl->sequence;
	smp_rmb();  // 保证先读取 sl->sequence 值，再读取被保护的值
	return ret;
}

/* Test if reader processed invalid data.
 * If initial values is odd, 
 *	then writer had already started when section was entered
 * If sequence value changed
 *	then writer changed data while in section
 *    
 * Using xor saves one conditional branch.
 */
static inline int read_seqretry(const seqlock_t *sl, unsigned iv)
{
	smp_rmb();  // 保证读取保护的值之后，再读取 sl->sequence
	return (iv & 1) | (sl->sequence ^ iv); // 两种情况下读到的值无效，一是读者开始读的时候，写者已经进行中，还未结束，此时 iv 为奇数(这就是为什么，写着需要更新两次的原因，通过 sl->sequence 的奇偶即可判断此种情况)
                                           // 另一种情况：读者读取过程中，有写者插入，此时 sl -> sequenece != iv
}
```
