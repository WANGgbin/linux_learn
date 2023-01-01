描述 linux 中的 TLS(Thread Local Storage).
linux 中,每个线程的 thread 中有3个 tls 段描述符.其定义为:
```c
# define GDT_ENTRY_TLS_ENTRIES 3
struct thread_struct{
    struct desc_struct tls_array[GDT_ENTRY_TLS_ENTRIES]; // 每个线程有3个 tls 描述符
};
```
实际上,每个 cpu 对应的 GDT(全局描述符表)中就有三个 entry 用于 tls. 当 linux 调度某个线程运行的时候,就会将线程 tls_array 中的值赋值到 GDT 中. 通过设置段寄存器 fs/gs,之后就可以通过这两个寄存器来访问线程 tls 数据了.

# 各平台 tls 的设置方法
那么该如何设置线程 tls 呢?不同的平台设置方法是不同的.
- linux+i386

linux+i386平台,可以通过系统调用`set_thread_area`来设置线程的 tls.函数原型为
```c
    #include<linux/unistd.h>
    #include<asm/ldt.h> # 定义 struct user_desc
    int set_thread_area(struct user_desc* user_info);
````
 注意,我们需要主动设置 fs/gs 寄存器的值,才可以通过 fs/gs 来访问 tls 数据. 可以参考:tls_i386.c 了解 set_thread_area 的使用.
- linux+x86

    在 linux+x86 平台上,通过系统调用 `arch_prctl` 来设置 fs/gs 段基地址. 函数原型为:
```c
    int arch_prctl(int option, long addr);
    // option 可选至为: 
    // 0X1001 ARCH_SET_GS 0X1004 ARCH_GET_GS
    // 0X1002 ARCH_SET_FS 0X1003 ARCH_GET_FS
```
该函数可以直接设置 fs/gs 段寄存器,我们可以直接通过 fs/gs 来访问 tls 数据. 详细使用可以参考:tls_x86.c

# tls 在 go 中的使用

go 在 `src/runtime/sys_linux_amd64.s` 中使用汇编编写了 `settls` 函数,该函数的功能就是通过调用 `arch_prctl` 将 m.tls[1] 的地址设置为 fs 段基地址. 我们分析下该函数的实现.
```s
TEXT runtime.settls(SB),NOSPLIT,$32
    ADDQ  $8, DI // DI 内容是 &m.tls[0], tls 是大小为 8字节数组,这里+8,使得 DI 等于 &m.tls[1]
    MOVQ $0X1002, SI // 设置 fs 段寄存器
    MOVQ $SYS_arch_prctl, AX  // 传递系统调用号
    SYSCALL  // 触发系统调用
    CMPQ AX, $0Xfffffffffffff001 // 判断系统调用结果
    JLS 2(PC)
    MOVL $0Xf1, 0xf1 // crash,因为 0xf1 是个非法地址,会发生 Segmentation fault.
    RET
```
在 x86 中,系统调用参数是通过寄存器传递的.
rax: 传递系统调用号
rdi: 第一个参数
rsi: 第二个参数
rdx: 第三个参数
r10: 第四个参数
r9: 第五个参数
r8: 第六个参数

settls 函数的调用是发生在 `src/runtime/asm_amd64.s`中. go 中是如何访问 tls 数据的呢?
通过函数 `get_tls`,该函数的实现,由 go 编译器在编译的时候才生成.大概是:
```s
    MOVQ TLS, BX
```
TLS 在 plan9 汇编中是一个虚拟寄存器,其底层对应的就是 fs 段寄存器,上述指令将 fs 对应的段基地址存储到 BX 中.

那么 go 在线程的 tls 中存储了什么数据呢?当前协程 g 的地址. 我们在 go runtime 中经常看到的函数 `getg()` 就是通过 tls 来获取当前 goroutine 的地址.