

#ifndef _CPU_H_
#define _CPU_H_


#include <spinlock.h>     // 包含自旋锁定义
#include <threadlist.h>   // 包含线程列表定义
#include <machine/vm.h>   /* for TLBSHOOTDOWN_MAX */


/*
 * Per-cpu 结构体 (每个 CPU 独立一份)
 *
 * 注意: curcpu 由 <current.h> 定义。
 *
 * 当 *使用* curcpu 的地址时（而不是仅仅解引用它），应该始终使用 cpu->c_self，
 * 以防 curcpu 被定义为一个具有固定地址和每个 CPU 映射的指针。
 */

struct cpu {
	/*
	 * 分配后固定不变的成员。
	 */
	struct cpu *c_self;		/* 此结构体的规范地址 */
	unsigned c_number;		/* 此 cpu 的软件编号 */
	unsigned c_hardware_number;	/* 硬件定义的 cpu 编号 */

	/*
	 * 仅由**当前 CPU** 访问的成员。
	 */
	struct thread *c_curthread;	/* CPU 上当前运行的线程 */
	struct threadlist c_zombies;	/* 已退出线程的列表 */
	unsigned c_hardclocks;		/* hardclock() 调用计数器 */
	unsigned c_spinlocks;		/* 持有的自旋锁计数器 */

	/*
	 * 被**其他 CPU** 访问的成员。
	 * 受 runqueue 锁保护。
	 */
	bool c_isidle;			/* 如果此 cpu 处于空闲状态，则为 True */
	struct threadlist c_runqueue;	/* 此 cpu 的运行队列 */
	struct spinlock c_runqueue_lock;

	/*
	 * 被**其他 CPU** 访问的成员。
	 * 受 IPI 锁保护。
	 *
	 * 发送到此 CPU 的 TLB 射击（shootdown）请求排队在 c_shootdown[] 中，
	 * c_numshootdown 存储请求数量。TLBSHOOTDOWN_MAX 是最大排队数量，
	 * 它是机器相关的。
	 *
	 * struct tlbshootdown 的内容也是机器相关的，可能是一个地址空间和
	 * 虚拟地址对，或一个物理地址，或其他。
	 */
	uint32_t c_ipi_pending;		/* 每个 IPI 编号对应一位 */
	struct tlbshootdown c_shootdown[TLBSHOOTDOWN_MAX];
	unsigned c_numshootdown;
	struct spinlock c_ipi_lock;

	/*
	 * 被**其他 CPU** 访问的成员。在 hangman.c 内部受保护。
	 */
	HANGMAN_ACTOR(c_hangman); // 用于调试/死锁检测
};

/*
 * 初始化函数。
 *
 * cpu_create 创建一个 cpu；适用于从驱动程序或总线特定的代码中调用，
 * 以查找次级 CPU。
 *
 * cpu_create 调用 cpu_machdep_init。
 *
 * cpu_start_secondary 是新 CPU 的平台相关汇编语言入口点；
 * 它可以在 start.S 中找到。它在声明了为该 cpu 创建的启动栈和线程后，
 * 调用 cpu_hatch。
 */
struct cpu *cpu_create(unsigned hardware_number);
void cpu_machdep_init(struct cpu *);
/*ASMLINKAGE*/ void cpu_start_secondary(void); // 汇编语言入口点
void cpu_hatch(unsigned software_number);       // CPU 启动完成后的 C 语言函数

/*
 * 生成描述 CPU 类型的字符串。
 */
void cpu_identify(char *buf, size_t max);

/*
 * 当前 CPU 的硬件级中断开启/关闭。
 *
 * 这些应该只由 spl 代码（软件优先级级别）使用。
 */
void cpu_irqoff(void); // 中断关闭
void cpu_irqon(void);  // 中断开启

/*
 * 使处理器空闲或关闭处理器。
 *
 * cpu_idle() 处于等待状态（如果可能，处于低功耗状态），直到它认为发生了
 * 有趣的事情，例如中断。然后它返回。（它可能判断错误，所以应该始终在
 * 检查其他条件的循环中调用。）调用时必须关闭中断以避免竞态条件，
 * 尽管中断可能会在它返回之前被传递。
 *
 * cpu_halt 处于等待状态（如果可能，处于低功耗状态），直到外部复位被按下。
 * 中断应该被禁用。它不会返回。它不应该允许中断被传递。
 */
void cpu_idle(void);
void cpu_halt(void);

/*
 * 处理器间中断 (Interprocessor interrupts, IPI)。
 *
 * 有时需要唤醒另一个 CPU。多处理器机器的系统板提供了这样做的方法。
 *
 * 当一个页被多个处理器映射在 MMU 中，并且该页正在被更改或需要
 * 在所有 CPU 上被失效时，VM 系统会执行 TLB 射击（shootdown）。
 *
 * ipi_send 向一个 CPU 发送 IPI。
 * ipi_broadcast 向除当前 CPU 之外的所有 CPU 广播 IPI。
 * ipi_tlbshootdown 类似于 ipi_send，但带有 TLB 射击数据。
 *
 * interprocessor_interrupt 在目标 CPU 接收到 IPI 时被调用。
 */

/* IPI 类型 */
#define IPI_PANIC		0	/* 系统调用了 panic() */
#define IPI_OFFLINE		1	/* 请求 CPU 下线 */
#define IPI_UNIDLE		2	/* 有可运行的线程可用 */
#define IPI_TLBSHOOTDOWN	3	/* MMU 映射需要失效 */

void ipi_send(struct cpu *target, int code);
void ipi_broadcast(int code);
void ipi_tlbshootdown(struct cpu *target, const struct tlbshootdown *mapping);

void interprocessor_interrupt(void);


#endif /* _CPU_H_ */
