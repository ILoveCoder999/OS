/*
 * 进程支持。
 * 这里（有意地）内容不多；你需要添加内容，可能还需要重新安排已有的内容。
 * p_lock 旨在操作进程结构中的指针时持有，而不是在对它们指向的内容进行
 * 任何重要工作时持有。根据需要重新安排这个（和/或将其更改为常规锁）。
 * 除非你实现多线程用户进程，否则唯一拥有多个线程的进程是内核进程。
 */
#include <types.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>

/*
 * 内核的进程；这包含所有仅内核线程。
 */
struct proc *kproc;

/*
 * 创建进程结构。
 */
 /*
static: 表示这个函数只在当前文件内可见
返回类型: struct proc * - 指向进程控制块的指针
参数: const char *name - 进程名称字符串
 */
static
struct proc *
proc_create(const char *name)
{
	struct proc *proc;
	/*
	kmalloc(): 内核内存分配函数（类似用户空间的malloc）
        sizeof(*proc): 计算struct proc结构体的大小
                 作用: 在内核堆中为新的进程控制块分配内存
                 错误处理: 如果内存不足返回NULL	
	*/
	// 分配进程结构内存
	proc = kmalloc(sizeof(*proc));
	if (proc == NULL) {
		return NULL;
	}
	/*
	kstrdup(name): 内核字符串复制函数
                  分配新内存
                  将传入的name字符串复制到新内存中
                  错误处理: 如果复制失败：
        kfree(proc): 释放之前分配的进程结构内存
                  返回NULL表示创建失败
        */
	
	// 复制进程名称
	proc->p_name = kstrdup(name);
	if (proc->p_name == NULL) {
		kfree(proc);
		return NULL;
	}
	
         
	// 初始化进程字段
	proc->p_numthreads = 0;           // 线程数为0
	spinlock_init(&proc->p_lock);     // 初始化进程锁
	
	//spinlock_init(): 初始化自旋锁
        //用途: 保护进程数据的并发fangwen
        //保护对象: 进程结构中的指针和计数器

	/* VM 字段 - 虚拟内存相关 
	初始值: NULL表示新进程还没有地址空间

后续: 当进程加载程序时会被设置为有效的地址空间
	
	*/
	proc->p_addrspace = NULL;         // 地址空间为空

	/* VFS 字段 - 文件系统相关 */
	proc->p_cwd = NULL;               // 当前工作目录为空

	return proc;
}

/*
 * 销毁进程结构。
 *
 * 注意：当前没有代码调用此函数。你的 wait/exit 代码可能需要这样做。
 */
void
proc_destroy(struct proc *proc)
{
	/*
	 * 希望在退出时销毁并清空大部分进程（特别是地址空间），
	 * 如果你的 wait/exit 设计要求进程结构在进程退出后继续存在。
	 * 有些 wait/exit 设计这样做，有些不这样做。
	 */

	// 断言：进程不能为空且不能是内核进程,jin cheng de xian cheng shu liang bi xu wei 0
	KASSERT(proc != NULL);
	KASSERT(proc != kproc);
	KASSERT(proc->p_numthreads == 0); 

	/*
	 * 我们在这里不获取 p_lock，因为我们必须是此结构的唯一引用。
	 * （否则销毁它是不正确的。）
	 */

	/* VFS 字段 - 文件系统清理 */
	if (proc->p_cwd) {
		VOP_DECREF(proc->p_cwd);  // 减少当前目录的引用计数
		proc->p_cwd = NULL;
	}

	/* VM 字段 - 虚拟内存清理 */
	if (proc->p_addrspace) {
		/*
		 * 如果 p 是当前进程，在销毁地址空间之前安全地将其从
		 * p_addrspace 中移除。这确保我们不会在地址空间正在
		 * 被销毁时尝试激活它。
		 *
		 * 同时显式停用，因为将地址空间设置为 NULL 不一定会这样做。
		 *
		 * （当地址空间为 NULL 时，表示进程仅是内核进程；在这种情况下，
		 * 如果 MMU 和 MMU 相关数据结构仍然引用最后一个有地址空间的
		 * 进程的地址空间，通常是正常的。然后如果该进程是下一个要运行的
		 * 进程（这并不罕见），你可以节省工作。但是，这里我们要销毁
		 * 地址空间，所以需要确保 VM 系统中没有任何东西仍然引用它。）
		 *
		 * 对 as_deactivate() 的调用必须在我们清除地址空间之后进行，
		 * 否则定时器中断可能会在我们背后重新激活旧的地址空间。
		 *
		 * 如果 p 不是当前进程，仍然在销毁之前将其从 p_addrspace 中移除
		 * 作为预防措施。注意，如果 p 不是当前进程，为了能在这里出现，
		 * p 必须要么从未运行过（例如，在 fork 失败后清理），要么已经
		 * 完成运行并退出。在某个随机其他进程仍在运行时销毁其进程结构
		 * 是完全不正确的...
		 */
		struct addrspace *as;

		if (proc == curproc) {
			// 当前进程：安全地设置地址空间并停用
			as = proc_setas(NULL);
			as_deactivate();
		}
		else {
			// 其他进程：直接获取并清空地址空间
			as = proc->p_addrspace;
			proc->p_addrspace = NULL;
		}
		as_destroy(as);  // 销毁地址空间
	}

	// 断言：进程应该没有线程了
	KASSERT(proc->p_numthreads == 0);
	spinlock_cleanup(&proc->p_lock);  // 清理自旋锁

	// 释放进程名称和结构内存
	kfree(proc->p_name);
	kfree(proc);
}

/*
 * 为内核创建进程结构。
 */
void
proc_bootstrap(void)
{
	kproc = proc_create("[kernel]");
	if (kproc == NULL) {
		panic("proc_create for kproc failed\n");
	}
}

/*
 * 为 runprogram 创建一个新的进程。
 *
 * 它将没有地址空间，并继承当前进程（即内核菜单）的当前目录。
 */
struct proc *
proc_create_runprogram(const char *name)
{
	struct proc *newproc;

	newproc = proc_create(name);
	if (newproc == NULL) {
		return NULL;
	}

	/* VM 字段 */
	newproc->p_addrspace = NULL;

	/* VFS 字段 */

	/*
	 * 锁定当前进程以复制其当前目录。
	 * （我们不需要锁定新进程，因为我们是它的唯一引用。）
	 */
	spinlock_acquire(&curproc->p_lock);
	if (curproc->p_cwd != NULL) {
		VOP_INCREF(curproc->p_cwd);      // 增加当前目录的引用计数
		newproc->p_cwd = curproc->p_cwd; // 共享当前目录
	}
	spinlock_release(&curproc->p_lock);

	return newproc;
}

/*
 * 向进程添加线程。线程或进程可能是也可能不是当前的。
 *
 * 在更改 t_proc 时关闭本地 CPU 的中断，以防它是当前线程，
 * 以保护定时器中断上下文切换中的 as_activate 调用，以及
 * 任何其他对"curproc"的隐式使用。
 */
int
proc_addthread(struct proc *proc, struct thread *t)
{
	int spl;

	KASSERT(t->t_proc == NULL);  // 断言线程还没有关联进程

	// 增加进程的线程计数
	spinlock_acquire(&proc->p_lock);
	proc->p_numthreads++;
	spinlock_release(&proc->p_lock);

	// 关闭中断并设置线程的进程关联
	spl = splhigh();
	t->t_proc = proc;
	splx(spl);

	return 0;
}

/*
 * 从进程中移除线程。线程或进程可能是也可能不是当前的。
 *
 * 在更改 t_proc 时关闭本地 CPU 的中断，以防它是当前线程，
 * 以保护定时器中断上下文切换中的 as_activate 调用，以及
 * 任何其他对"curproc"的隐式使用。
 */
void
proc_remthread(struct thread *t)
{
	struct proc *proc;
	int spl;

	proc = t->t_proc;
	KASSERT(proc != NULL);  // 断言线程有关联的进程

	// 减少进程的线程计数
	spinlock_acquire(&proc->p_lock);
	KASSERT(proc->p_numthreads > 0);
	proc->p_numthreads--;
	spinlock_release(&proc->p_lock);

	// 关闭中断并清除线程的进程关联
	spl = splhigh();
	t->t_proc = NULL;
	splx(spl);
}

/*
 * 获取（当前）进程的地址空间。
 *
 * 注意：地址空间没有引用计数。如果你实现多线程进程，
 * 请确保设置引用计数方案或其他方法使此操作安全。
 * 否则返回的地址空间可能会在你使用时消失。
 */
struct addrspace *
proc_getas(void)
{
	struct addrspace *as;
	struct proc *proc = curproc;

	if (proc == NULL) {
		return NULL;
	}

	// 安全地获取地址空间指针
	spinlock_acquire(&proc->p_lock);
	as = proc->p_addrspace;
	spinlock_release(&proc->p_lock);
	return as;
}

/*
 * 更改（当前）进程的地址空间。返回旧的地址空间以供后续恢复或处置。
 */
struct addrspace *
proc_setas(struct addrspace *newas)
{
	struct addrspace *oldas;
	struct proc *proc = curproc;

	KASSERT(proc != NULL);  // 断言当前进程不为空

	// 安全地设置新的地址空间
	spinlock_acquire(&proc->p_lock);
	oldas = proc->p_addrspace;
	proc->p_addrspace = newas;
	spinlock_release(&proc->p_lock);
	return oldas;
}
