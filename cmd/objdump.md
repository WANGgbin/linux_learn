描述 objdump 的使用. objdump 用来查看一个二进制文件的内容.

# option
- -h

-h, --section-headers, --headers 这三个选项等价.

-h 用来查看文件的节头部表,节头部表中的信息就是各个节的描述信息.包括:其实地址,长度,权限.
eg: 我们查看一个 go 可执行文件的节头部表.
```text
main:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         0007e8da  0000000000401000  0000000000401000  00001000  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .rodata       000352a5  0000000000480000  0000000000480000  00080000  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  2 .typelink     000004d8  00000000004b5440  00000000004b5440  000b5440  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .itablink     00000058  00000000004b5920  00000000004b5920  000b5920  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .gosymtab     00000000  00000000004b5978  00000000004b5978  000b5978  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  5 .gopclntab    000597b8  00000000004b5980  00000000004b5980  000b5980  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  6 .go.buildinfo 00000020  0000000000510000  0000000000510000  00110000  2**4
                  CONTENTS, ALLOC, LOAD, DATA
  7 .noptrdata    000105c0  0000000000510020  0000000000510020  00110020  2**5
                  CONTENTS, ALLOC, LOAD, DATA
  8 .data         00007810  00000000005205e0  00000000005205e0  001205e0  2**5
                  CONTENTS, ALLOC, LOAD, DATA
  9 .bss          0002ef08  0000000000527e00  0000000000527e00  00127e00  2**5
                  ALLOC
 10 .noptrbss     00005360  0000000000556d20  0000000000556d20  00156d20  2**5
                  ALLOC
...
```

- -p

-p 用来查看二进制文件的段头部表,段头部表内容就是二进制文件如何映射到进程的虚拟地址空间.

```text

main:     file format elf64-x86-64

Program Header:
    PHDR off    0x0000000000000040 vaddr 0x0000000000400040 paddr 0x0000000000400040 align 2**12
         filesz 0x0000000000000188 memsz 0x0000000000000188 flags r--
    NOTE off    0x0000000000000f9c vaddr 0x0000000000400f9c paddr 0x0000000000400f9c align 2**2
         filesz 0x0000000000000064 memsz 0x0000000000000064 flags r--
    LOAD off    0x0000000000000000 vaddr 0x0000000000400000 paddr 0x0000000000400000 align 2**12
         filesz 0x000000000007f8da memsz 0x000000000007f8da flags r-x
    LOAD off    0x0000000000080000 vaddr 0x0000000000480000 paddr 0x0000000000480000 align 2**12
         filesz 0x000000000008f138 memsz 0x000000000008f138 flags r--
    LOAD off    0x0000000000110000 vaddr 0x0000000000510000 paddr 0x0000000000510000 align 2**12
         filesz 0x0000000000017e00 memsz 0x000000000004c080 flags rw-
   STACK off    0x0000000000000000 vaddr 0x0000000000000000 paddr 0x0000000000000000 align 2**3
         filesz 0x0000000000000000 memsz 0x0000000000000000 flags rw-
0x65041580 off    0x0000000000000000 vaddr 0x0000000000000000 paddr 0x0000000000000000 align 2**3
         filesz 0x0000000000000000 memsz 0x0000000000000000 flags --- 2a00
```

- -f

-f 用来查看 elf 头,包括一些平台相关信息以及程序入口地址.

```text
main:     file format elf64-x86-64  // 文件格式
architecture: i386:x86-64, flags 0x00000112:  // 平台信息
EXEC_P, HAS_SYMS, D_PAGED
start address 0x000000000045c220 // 入口地址
```

- -t

-t, --syms 等价.
-t 表示二进制文件的所有符号信息.

```text

main:     file format elf64-x86-64

SYMBOL TABLE:
// 起始地址         flag   所在分节名字   大小       符号名字
0000000000000000 l    df *ABS*  0000000000000000 go.go
0000000000401000 l     F .text  0000000000000000 runtime.text
0000000000401d40 l     F .text  000000000000022d cmpbody
0000000000401fa0 l     F .text  000000000000013e memeqbody
0000000000402120 l     F .text  0000000000000117 indexbytebody
0000000000457c60 l     F .text  0000000000000040 gogo
```
其中, flag 字段是一系列空格分割的字符集合,常见的符号有:
 - l local
 - g global
 - F function
 - O object

- -d

-d, --disassemble 等价
-d 查看可执行分节的反汇编,主要就是 .text 的反汇编代码.

- -D

-d, --disassemble-all 等价
查看所有分节的反汇编内容.

- -S

与 -d 类似,源码跟反汇编交互

- -j

-j, --section 等价
-j 查看特定分节内容,注意 -j 不能单独使用! 一般跟 -D或者-S 结合使用. 
