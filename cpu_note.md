**本文描述 cpu 相关特性，包括：指令流水线、乱序、内存屏障、cache 一致性协议(MESI)、cache 原理等。**
***
## 指令流水线
## 乱序
乱序有两类。第一类是**乱序执行**，第二类是 cpu 观察乱序。
第一类乱序好理解，为了提高指令并发度，对于一些数据相关的指令可以推迟执行，先执行其他的指令。
第二类乱序如何理解呢？在多核系统中，即使某个核按序执行指令，但是站在其他 cpu 的角度看，得到的结果似乎是乱序的。
为什么按序执行，其他 cpu 看到的结果却是乱序的呢？这其实是硬件优化导致的。为了提高硬件的性能，硬件有各种各样的优化，比如典型的 store buffer。在 store 写内存的时候，如果未命中 cache，则会先写入到 store buffer，后续在合适的时机在同步到 cache 以及内存中。如果优化会影响到结果的正确性，那么这样的优化实际上是没有意义的。但是在当前 cpu 的角度看，这些优化总能得到正确的结果。比如写入到 store buffer，当前 cpu 在 load 的时候，会优先从 store buffer 读取。但是，如果某些数据是多核共享的，就会存在乱序的问题。显然，多核访问数据(**多线程通过锁同步不属于此种情况，因为任何时候，只会在某一个 cpu 上访问数据**)属于较少的情况，所以硬件选择了较少错误率的情况下选择了优化。这实际上是错误率与性能的斟酌。当然优化导致的错误是可以通过某种方式解决的(**如果解决不了，优化是没有意义的**)。如何解决呢？这就是内存屏障存在的意义！
比如常见的[store-store乱序](https://zhuanlan.zhihu.com/p/143297327)。
```c
int flag, data;


static void write_data_cpu0(int value)
{
        data = value;
        flag = 1;
}


static void read_data_cpu1(void)
{
        while (flag == 0);
        return data;
}
```
假如 flag 已经缓存到 cpu0 的 cache 中了，在执行 `data = value` 的时候，结果写入到 store buffer 中，接着执行`flag = 1` 的时候，flag 更新到 cache 中了。这导致的结果是其他 cpu 只能看到 flag 的值，只有到 data 写入到 cpu0 的cache 后，其他 cpu 才可以看到 data 的值。
接着 cpu1 读到 `flag = 1` 跳出循环，然后读取 data 的值，此时读到的 data 是空的，此时站在 cpu1 的角度看， cpu0 中两条语句的执行似乎是乱序的，先执行了 flag = 1，实际并非如此。
那么如何解决呢？使用写屏障。linux 内核中提供了 `smp_wmb()` 对不同架构的屏障指令进行了封装，有些平台是`wfence`汇编指令。写屏障保证了，data = value 写内存一定在 flag = 1 之前，从而解决了乱序的问题。那么`wfence` 又是如何解决乱序的呢？这其实属于硬件层面的内容，我们不应该太过关注，作为软件工程师，可以把乱序当做一个黑盒，明白存在乱序的问题以及知道如何解决乱序的问题即可。

除了`store-store乱序`还有[load-load乱序](https://zhuanlan.zhihu.com/p/155966754)、[store-load乱序](https://zhuanlan.zhihu.com/p/141655129)
## 内存屏障
在乱序一节已经介绍过屏障相关的内容，这里通过回答几个问题，对屏障内容进行总结。参考：[如何使用屏障指令](https://zhuanlan.zhihu.com/p/346352307)
- 什么时候使用屏障？
    存在多核同时访问数据！如果存在数据竞争，但是任何时刻只有一个 cpu 在访问数据(通过锁等机制)则没必要使用屏障。
- 使用什么类型的屏障？
    就三类屏障，smp_mb、smp_wmb、smp_rmb，按需使用！
    > 一个使用内存屏障的典型例子是linux内核中顺序锁的实现


## atomic 实现
参考[atomic 实现原理](https://zhuanlan.zhihu.com/p/115355303)
两种实现方式：
- 锁 bus
- 锁 cache

# cache
本小节描述 cache(高速缓存) 相关内容。
## [cache 基本原理](https://zhuanlan.zhihu.com/p/102293437)
我们从三个角度来描述 cache。
- cache line 分配策略
- cache line 更新策略
- cache line 映射策略

分配策略主要指的是当写 miss 的时候，如何分配 cache line. 常见的策略有`写分配`和`未写分配`。写分配指的是，会分配一个 cache line，然后从主存中读取数据到 cache line，最后再更新 cache line。
未写分配指的是不分配 cache line，直接写入到主寸中(准确说是下一级存储)。读操作 miss 的时候，是一定会分配 cache line 的。

更新策略指的是 cache hit 的时候，如何更新 cache。常见的策略有`直写`和`写回`。直写指的是除了更新 cache line 外，还会同步更新到主存中。这种方式实现简单，但是性能比较差。写回指的是只更新 cache line，
在合适的时机，比如 cache line 驱逐的时候才更新到主存中，这种方式性能较好，但是实现比较复杂。

映射策略指的是 cache 控制器如何根据地址(**可能是物理地址、可能是虚拟地址，由实现决定**)找到对应的 cache line。 常见的映射策略有`直接映射`、`组相联映射`、`全相联映射`。其实这三种方式是同一种方式，直接映射每个组只有一个 cache line，全相联映射只有一个组，这两种方式都可以看作是特殊的组相联映射。

## cache 对代码的影响
要书写 cache 友好地代码，就要尽量提高 cache 命中率，要防止 cache 抖动：频繁加载/驱逐相同的高速缓存行。具体的例子可以参考 csapp.6.5

## cache 的组织方式
## cache 一致性协议
参考链接：
+ [10张图打开 CPU 缓存一致性的大门](https://www.cnblogs.com/xiaolincoding/p/13886559.html)
+ [多核 cache 一致性](https://zhuanlan.zhihu.com/p/115114220)
> 注意，写入 store buffer 的时候，是不会触发 cache 一致性协议的！因为并没有写入到 cache，MESI 只用来保证 cache 的一致性！


