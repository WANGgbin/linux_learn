#include<linux/unistd.h>
#include<asm/ldt.h>
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<sys/syscall.h>

struct tls_data {
    struct tls_data * self; // 注意第一项需要设置为指向自己的地址,这样才可以通过 movq gs:0 来获取到 tls 地址
    int data;
};

// 通过汇编自定义的系统调用. 当 glibc 没有提供某个系统调用的 wrapper 的时候,我们可以通过
// syscall 来调用系统调用,或者自己通过汇编来封装系统调用.
int my_arch_prctl(struct tls_data* ptr) {
    int ret = -1;
    __asm__ __volatile(
        "movq $0x1001, %%rdi\n\t" // 注意立即数寻址
        "movq %1, %%rsi\n\t"
        "movl %2, %%eax\n\t"
        "syscall":"=a"(ret):"m"(ptr),"i"(SYS_arch_prctl));
}

void pthread_tls_init() {
    printf("Entering pthread_tls_init\n");
    struct tls_data* data = (struct tls_data*)malloc(sizeof(struct tls_data));
    memset(data, 0, sizeof(struct tls_data));
    data->data = 1;
    data->self = data;
    // i386 是支持 set_thread_area 系统调用的, X86-64 不支持此系统调用
    struct user_desc desc;
    memset(&desc, 0, sizeof(struct user_desc));
    desc.entry_number = -1; // 当 entry_number 设置为 -1 的时候,内核会自己寻找线程空闲的 tls entry, 并将 entry_number 设置到此成员中
    desc.base_addr = (unsigned int)data; // 基地址
    desc.limit = 0xfffff; // 段大小
    desc.seg_32bit = 1;
    desc.limit_in_pages = 1; // limit 单位,字节还是页
    desc.useable = 1;
  
    printf("start to set thread_area\n");
    int ret = syscall(SYS_set_thread_area,&desc);
    if(ret == -1) {
        perror("set_thread_area error");
        exit(errno);
    }
    printf("tls data was set in entry_number: %d\n", desc.entry_number);

    // 设置 gs 寄存器
    __asm__ __volatile__ ("movl %0, %%gs"::"a"(desc.entry_number<<3+3));
    // 获取 gs 寄存器
    long gs = 0;
    __asm__ __volatile__ (
        "movq %%gs:0, %0"
        :"=a"(gs)
    );
    printf("gs: %p\n", (void*)gs);
}

// get_tls 获取 tls 地址
struct tls_data* get_tls() {
    struct tls_data* ptr = 0;
    __asm__ __volatile__("movq %%gs:0, %0":"=a"(ptr));

    if(ptr == 0) {
        printf("get tls_data error");
        exit(-1);
    }
    return ptr;
}

// 设置 tls 数据
void pthread_set_tls_int(int data) {
    get_tls()->data = data;
}

// 获取 tls 数据
int pthread_get_tls_int() {
    return get_tls()->data;
}


int main() {
    pthread_tls_init();
    pthread_set_tls_int(1);
    int value = pthread_get_tls_int();
    printf("got value: %d", value);
    return 0;
}