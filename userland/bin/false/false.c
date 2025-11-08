#include <unistd.h>
#include <stdlib.h>

/*
 * false - 总是返回失败的命令
 *
 */

int
main(void)
{
	/* ========== 执行流程：直接退出并返回失败状态 ========== */
	
	// 使用 exit(1) 立即终止程序，并向操作系统返回状态码 1
	// 在 Unix/Linux 系统中：
	// - 返回 0 表示成功 (success)
	// - 返回非0 表示失败 (failure)
	exit(1);
	//exit 	is system call
	// 程序执行到这里就结束了，不会继续运行
}


