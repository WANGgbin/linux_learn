# 页高速缓存
## flush 内核线程设计
linux 内核的页刷新内核线程的数量是动态调整过的，如何增加或者减少内核线程数量呢？
> linux version: linux-2.6.9/mm/pdflush.c

- 首先初始化两个工作线程，怎么初始化的呢？先通过`kthread_run`创建线程，然后在线程内部阻塞。
> 这是经典的实现，先创建再阻塞
```c
static int __init pdflush_init(void)
{
	int i;

	for (i = 0; i < MIN_PDFLUSH_THREADS; i++)
		start_one_pdflush_thread();
	return 0;
}

static void start_one_pdflush_thread(void)
{
	kthread_run(pdflush, NULL, "pdflush");
}
```
