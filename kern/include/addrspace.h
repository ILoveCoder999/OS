
/*
addrspace.h 是一个操作系统内核头文件，用于定义进程的**地址空间（Address Space）**结构体及其相关的操作函数。地址空间是操作系统中最重要的概念之一，它定义了一个程序运行时所能“看到”的虚拟内存布局
*/
/*
struct addrspace 封装了一个进程的所有内存管理信息。每个进程都会拥有一个独立的地址空间结构体，它告诉内核和硬件（通过页表）如何将程序的虚拟地址映射到物理地址

*/
#ifndef _ADDRSPACE_H_
#define _ADDRSPACE_H_

/*
 * 地址空间结构体和操作。
 */


#include <vm.h>           // 虚拟内存相关定义
#include "opt-dumbvm.h"   // 包含对“dumbvm”可选配置的引用

struct vnode;             // 文件系统中的虚拟节点结构体声明


/*
 * 地址空间 (Address space) - 与进程的虚拟内存空间相关联的数据结构。
 *
 * 需要你来编写实现。
 */

struct addrspace {
/*
OPT_DUMBVM 分支（简易 VM）：
这段代码提供了一个**简易虚拟内存（Dumb VM）**的实现（通常用于教学或简单内核的启动阶段）。
它将进程的地址空间划分为几个固定的、连续的区域：区域 1（通常是代码和全局数据）、区域 2（通常是堆）和栈。
vaddr_t (Virtual Address) 和 paddr_t (Physical Address) 分别存储这些区域的虚拟基地址和对应的物理基地址。as_npagesX 记录了区域的大小（以页为单位）
*/
#if OPT_DUMBVM
        // 简易（dumb）虚拟内存系统 (DUMBVM) 的成员：
        vaddr_t as_vbase1;      // 第一个内存区域（例如代码/数据）的虚拟基地址
        paddr_t as_pbase1;      // 第一个内存区域的物理基地址
        size_t as_npages1;      // 第一个内存区域的页数
        vaddr_t as_vbase2;      // 第二个内存区域（例如堆）的虚拟基地址
        paddr_t as_pbase2;      // 第二个内存区域的物理基地址
        size_t as_npages2;      // 第二个内存区域的页数
        paddr_t as_stackpbase;  // 栈区域的物理基地址
#else
        /* 在这里放置你的 VM 系统的结构体成员 */
        /*
                 如果不用简易 VM，内核开发者需要在这里定义更复杂的结构体成员，例如页表（Page Table）的根指针、段表或区域列表等，以支持更灵活的内存管理和分页
        */
#endif
};
//函数定义了地址空间的整个生命周期和与 CPU 的交互。
/*
 * addrspace.c 中的函数:
 *
 * as_create - 创建一个新的空地址空间。你需要确保它在所有正确的
 * 位置被调用。你 can修改参数列表。内存不足时可能返回 NULL。
 *
 * as_copy   - 创建一个与旧地址空间完全相同的副本。可能通过调用 as_create 
 * 获取一个新的空地址空间并填充它，这取决于你。
 *
 * as_activate -将当前线程的地址空间加载到 CPU 的内存管理单元（如 MIPS 上的 TLB），使处理器开始使用这个地址空间进行地址翻译。地址翻译（Address Translation），或者更正式地说，虚拟地址到物理地址的转换，是操作系统和计算机硬件协同完成的核心任务，用于实现虚拟内存.这是上下文切换（Context Switch）的关键一步。
 *
 * as_deactivate - 卸载当前的地址空间。在地址空间被销毁之前（例如进程退出时）调用，以确保硬件不再尝试访问它。
 *
 * as_destroy - 销毁一个地址空间。如果实现了用户级线程，可能需要更改此函数的行为。
 *
 区域定义函数（加载可执行文件）
这些函数用于在地址空间内划分和设置内存区域，主要用于加载可执行文件（如 ELF 格式）
 * as_define_region - 在地址空间内设置一个内存区域。
 *在地址空间 as 中定义一个内存区域（例如代码段、数据段）。参数包括虚拟地址 (vaddr)、大小 (sz) 和权限（可读 readable、可写 writeable、可执行 executable）。
 * as_prepare_load - 在实际从可执行文件加载数据到地址空间之前调用。
 在实际加载可执行文件数据之前调用。例如，可以在这里锁定地址空间结构，或执行必要的初始化操作
 *
 * as_complete_load - 从可执行文件加载完成后调用。
 在可执行文件的数据加载完成后调用。例如，可以在这里设置只读权限，或将任何未使用的区域清理干净。
 *
 * as_define_stack - 在地址空间中设置栈区域。
 * （通常在 as_complete_load() *之后* 调用。）返回新进程的初始栈指针。
 在地址空间的顶部设置栈区域。它还会返回新进程的初始栈指针（initstackptr），这是程序开始执行所必需的。
 *
 * 注意：当使用 dumbvm 时，addrspace.c 不被使用，这些函数在 dumbvm.c 中。
 */

struct addrspace *as_create(void);
int               as_copy(struct addrspace *src, struct addrspace **ret);
void              as_activate(void);
void              as_deactivate(void);
void              as_destroy(struct addrspace *);

int               as_define_region(struct addrspace *as,
                                   vaddr_t vaddr, size_t sz,
                                   int readable,
                                   int writeable,
                                   int executable);
int               as_prepare_load(struct addrspace *as);
int               as_complete_load(struct addrspace *as);
int               as_define_stack(struct addrspace *as, vaddr_t *initstackptr);


/*
 * loadelf.c 中的函数
 * load_elf - 将一个 ELF 格式的用户程序可执行文件加载到当前地址空间。
 * 将入口点（初始 PC）返回到 ENTRYPOINT 指向的空间中。
 这是加载器函数，负责读取一个存储在文件系统中的 ELF 格式可执行文件（文件描述符由 struct vnode *v 表示），并将其内容映射到当前进程的地址空间中。

结果： 成功后，它会返回程序开始执行的虚拟地址（即入口点/Initial PC），并通过 entrypoint 指针传出
 */

int load_elf(struct vnode *v, vaddr_t *entrypoint);
/*
加载”可执行文件主要由 load_elf() 函数在内核中完成
1 定义虚拟内存区域 (Setup VA)
在实际移动任何数据之前，内核会根据可执行文件（ELF）的头信息，使用 as_define_region() 等函数，在进程的地址空间中创建和标记所需的虚拟内存区域（如代码段、数据段）
2 从磁盘复制到物理内存 (Copy Data)
读取文件： 内核读取可执行文件中相应段（Section）的数据（例如程序指令和初始化的全局变量）。
分配物理页： 内核通过 VM 系统分配足够的物理内存页（Page Frames）来存放这些数据。
复制数据： 将读取到的文件内容复制到新分配的物理内存页中
3设置地址翻译映射 (Setup Mapping)
这是关键的一步，也是加载与简单复制的最大区别：

建立映射： 内核更新进程的页表，创建从虚拟地址（进程代码中使用的地址）到物理地址（数据实际存放的 RAM 位置）的映射关系。

设置权限： 根据 ELF 文件和 as_define_region() 的调用，设置每个内存区域的权限（例如：代码段是可读、可执行但不可写；数据段是可读可写）

数据复制： 是将磁盘文件中的数据复制到 RAM 中。
结构创建： 是创建和配置一套新的虚拟内存管理结构（页表、地址空间结构体），用于管理这些数据，并为 CPU 做好地址翻译的准备。

*/


#endif /* _ADDRSPACE_H_ */
