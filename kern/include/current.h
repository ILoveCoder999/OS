// current cpu thread and proc

#ifndef _CURRENT_H_
#define _CURRENT_H_

/*
 * curcpu 和 curthread 的定义。
 *
 * 机器相关的头文件应该将 curcpu 或 curthread 定义为宏（但不能同时定义两者）；
 * 然后我们使用其中一个来获取另一个，并包含所需头文件以进行该引用。
 * （这些 include 语句是该文件没有被合并到 cpu.h 或 thread.h 中的原因。）
 *
 * 这部分材料是机器依赖的，因为在某些平台上，最好/更容易跟踪 curcpu 
 * 并让 curthread 为 curcpu->c_curthread，而在其他平台上，最好跟踪 curthread 
 * 并让 curcpu 为 curthread->t_cpu。
 *
 * 无论哪种方式，我们都不希望检索 curthread 或 curcpu 是昂贵的；
 * 在系统板寄存器中搜索和诸如此类的操作不是一个好主意。
 * 因此，我们希望以某种方式将 curthread 或 curcpu 保存在芯片上的某个地方。
 *
 * 有各种可能的方法；例如，可以使用每个 CPU 上的 MMU 将该 CPU 的 cpu 结构体
 * 映射到一个在所有 CPU 上都相同的固定虚拟地址。然后 curcpu 可以是一个常量。
 * （但必须记住在任何对其他 CPU 可见的地方使用 curcpu->c_self 作为指针的规范形式。）
 * 在某些 CPU 上，CPU 编号或 cpu 结构体基地址可以存储在 supervisor-mode 寄存器中，
 * 可以在启动期间设置好，然后保持不变。另一种方法是保留一个寄存器来保存 curthread，
 * 并在上下文切换期间更新它。
 *
 * 请参阅每个平台的 machine/current.h，了解其做法和原因。
 */

#include <machine/current.h.h> // 包含机器相关的定义（定义 __NEED_CURTHREAD 或 __NEED_CURCPU）

#if defined(__NEED_CURTHREAD)

#include <cpu.h>
#define curthread curcpu->c_curthread // 通过 curcpu 获取 curthread
#define CURCPU_EXISTS() (curcpu != NULL)

#endif

#if defined(__NEED_CURCPU)

#include <thread.h>
#define curcpu curthread->t_cpu // 通过 curthread 获取 curcpu
#define CURCPU_EXISTS() (curthread != NULL)

#endif

/*
 * curproc 的定义。
 *
 * curproc 始终是当前线程的进程。
 */

#define curproc (curthread->t_proc)


#endif /* _CURRENT_H_ */
