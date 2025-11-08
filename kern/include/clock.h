

#ifndef _CLOCK_H_
#define _CLOCK_H_

/*
 * 时间相关的定义。
 */

#include <kern/time.h> // 包含 struct timespec 等定义


/*
 * hardclock() 在每个 CPU 上每秒被调用 HZ 次，可能只在 CPU 不空闲时调用，用于调度。
 */

/* 每秒 hardclock 调用的次数 */
#define HZ  100

void hardclock_bootstrap(void); // hardclock 的初始化
void hardclock(void);           // 每次时钟中断时调用的函数（用于调度）

/*
 * timerclock() 每秒在单个 CPU 上调用一次，用于简单的定时操作。
 * （这是一个相当简单化的接口。）
 */
void timerclock(void);

/*
 * gettime() 可用于获取当前的日历时间。
 */
void gettime(struct timespec *ret);

/*
 * 时间算术运算
 *
 * add: ret = t1 + t2
 * sub: ret = t1 - t2
 */

void timespec_add(const struct timespec *t1,
		  const struct timespec *t2,
		  struct timespec *ret);
void timespec_sub(const struct timespec *t1,
		  const struct timespec *t2,
		  struct timespec *ret);

/*
 * clocksleep() 暂停执行所请求的秒数，类似于用户级的 sleep(3)。
 * （不要与 wchan_sleep 混淆。）
 */
void clocksleep(int seconds);


#endif /* _CLOCK_H_ */
