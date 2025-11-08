#ifndef _PROC_H_
#define _PROC_H_

/*
 * 进程定义
 * 
 * 注意：curproc 在 <current.h> 中定义
 */

#include <spinlock.h>
struct addrspace;
struct thread;
struct vnode;
/*
 * 进程结构体
 * 
 * 注意：我们只统计每个进程中的线程数量。
 * （除非你实现多线程用户进程，否则这个数字除了在内核进程中不会超过1）
 * 如果你想知道进程中具体有哪些线程（例如用于调试），可以添加一个数组
 * 和一个保护该数组的睡眠锁。（不能使用自旋锁保护数组，因为数组可能需要调用kmalloc）
 * 
 * 你很可能会向这个结构体添加更多内容，所以可能会发现由于其他原因也需要在这里使用睡眠锁。
 * 但是，请注意 p_addrspace 必须由自旋锁保护：
 * thread_switch 需要能够在不睡眠的情况下获取当前地址空间。
 */
struct proc {
    char *p_name;                   /* 进程名称 */
    struct spinlock p_lock;         /* 保护该结构体的锁 */
    unsigned p_numthreads;          /* 该进程中的线程数量 */

    /* 虚拟内存相关 */
    struct addrspace *p_addrspace;  /* 虚拟地址空间 */

    /* 虚拟文件系统相关 */
    struct vnode *p_cwd;            /* 当前工作目录 */

    /* 根据需要在此添加更多内容 */
};

/* 这是内核和仅内核线程的进程结构 */
extern struct proc *kproc;

/* 系统启动时调用一次，用于分配数据结构 */
void proc_bootstrap(void);

/* 为 runprogram() 创建一个新的进程 */
struct proc *proc_create_runprogram(const char *name);

/* 销毁一个进程 */
void proc_destroy(struct proc *proc);

/* 将线程附加到进程。该线程必须还没有关联的进程 */
int proc_addthread(struct proc *proc, struct thread *t);

/* 从进程中分离线程 */
void proc_remthread(struct thread *t);

/* 获取当前进程的地址空间 */
struct addrspace *proc_getas(void);

/* 更改当前进程的地址空间，并返回旧的地址空间 */
struct addrspace *proc_setas(struct addrspace *);

#endif /* _PROC_H_ */
