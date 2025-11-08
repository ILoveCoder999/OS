#include <unistd.h>
#include <err.h>

/*
 * cp - 文件复制命令
 * 用法: cp 源文件 目标文件
 */

/* 复制文件函数 */
static
void
copy(const char *from, const char *to)
{
	int fromfd;     // 源文件描述符
	int tofd;       // 目标文件描述符
	char buf[1024]; // 1KB 数据缓冲区
	int len, wr, wrtot;

	/* ========== 执行流程第1步：打开源文件 ========== */
	// 以只读方式打开源文件
	fromfd = open(from, O_RDONLY);
	if (fromfd < 0) {
		err(1, "%s", from);  // 如果打开失败，报错并退出
	}

	/* ========== 执行流程第2步：创建目标文件 ========== */
	// 以写入方式打开目标文件，如果不存在则创建，如果存在则清空
	tofd = open(to, O_WRONLY|O_CREAT|O_TRUNC);
	if (tofd < 0) {
		err(1, "%s", to);    // 如果创建失败，报错并退出
	}

	/* ========== 执行流程第3步：数据复制循环 ========== */
	/*
	 * 循环读取源文件数据：
	 * len > 0: 成功读取到数据
	 * len = 0: 到达文件结尾(EOF)
	 * len < 0: 读取错误
	 */
	while ((len = read(fromfd, buf, sizeof(buf))) > 0) {
		/*
		 * 循环写入目标文件：
		 * 由于write可能不会一次性写完所有数据，需要循环直到全部写入
		 */
		wrtot = 0;  // 已写入字节数计数器
		while (wrtot < len) {
			// 写入剩余数据：buf + wrtot 指向未写入数据的起始位置
			wr = write(tofd, buf + wrtot, len - wrtot);
			if (wr < 0) {
				err(1, "%s", to);  // 写入错误，报错退出
			}
			wrtot += wr;  // 更新已写入字节数
		}
	}

	/* ========== 执行流程第4步：错误检查 ========== */
	// 检查读取过程中是否发生错误
	if (len < 0) {
		err(1, "%s", from);  // 读取错误，报错退出
	}

	/* ========== 执行流程第5步：关闭文件并检查 ========== */
	// 关闭源文件描述符
	if (close(fromfd) < 0) {
		err(1, "%s: close", from);  // 关闭失败，报错退出
	}

	// 关闭目标文件描述符
	if (close(tofd) < 0) {
		err(1, "%s: close", to);    // 关闭失败，报错退出
	}
}

int
main(int argc, char *argv[])
{
	/*
	 * ========== 程序总执行流程 ==========
	 * 1. 参数验证 → 2. 调用复制函数 → 3. 返回退出
	 */
	
	/* ========== 执行流程第1步：参数验证 ========== */
	// 检查命令行参数，必须是3个：程序名、源文件、目标文件
	if (argc != 3) {
		errx(1, "Usage: cp OLDFILE NEWFILE");  // 参数错误，显示用法并退出
	}

	/* ========== 执行流程第2步：执行复制操作 ========== */
	copy(argv[1], argv[2]);  // 调用复制函数，argv[1]=源文件，argv[2]=目标文件

	/* ========== 执行流程第3步：程序正常结束 ========== */
	return 0;  // 复制成功，返回0表示正常退出
}
