#include<linux/unistd.h>
#include<asm/ldt.h>
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<sys/syscall.h>

struct tls_data {
    struct tls_data * self;
    int data;
};

// 通过汇编自定义的系统调用. 当 glibc 没有提供某个系统调用的 wrapper 的时候,我们可以通过
// syscall 来调用系统调用,或者自己通过汇编来封装系统调用.
int my_arch_prctl(struct tls_data* ptr) {
    int ret = -1;
    __asm__ __volatile(
        "movq $0x1001, %%rdi\n\t" // 注意立即数寻址
        "movq %1, %%rsi\n\t" // 多条指令的话,每条指令使用""包裹,注意除最后一条指令外,其他指令都要加上'\n\t'
        "movl %2, %%eax\n\t"
        "syscall":"=a"(ret):"m"(ptr),"i"(SYS_arch_prctl));
}

void pthread_tls_init() {
    printf("Entering pthread_tls_init\n");
    struct tls_data* data = (struct tls_data*)malloc(sizeof(struct tls_data));
    memset(data, 0, sizeof(struct tls_data));
    data->data = 1;
    data->self = data;
  
    printf("start to set thread_area\n");
    // 以下两条语句是等价的.
    int ret = my_arch_prctl(data);
    // int ret = syscall(SYS_arch_prctl, 0x1001, data);
    if(ret == -1) {
        perror("set_thread_area error");
        exit(errno);
    }
    printf("set_thread_area result: %d\n", ret);

    long addr = 0;
    ret = syscall(SYS_arch_prctl, 0x1004, &addr);
    if(ret == -1) {
        perror("get_fs error");
        exit(1);
    }
    printf("data: %p, addr: %p\n", data, (void*)addr);
}

struct tls_data* get_tls() {
    struct tls_data* ptr = 0;
    __asm__ __volatile__("movq %%gs:0, %0":"=a"(ptr));

    if(ptr == 0) {
        printf("get tls_data error");
        exit(-1);
    }
    return ptr;
}

void pthread_set_tls_int(int data) {
    get_tls()->data = data;
}

int pthread_get_tls_int() {
    return get_tls()->data;
}


int main() {
    pthread_tls_init();
    pthread_set_tls_int(2);
    int value = pthread_get_tls_int();
    printf("got value: %d", value);
     return 0;
}