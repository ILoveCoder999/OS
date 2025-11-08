#include <unistd.h>
#include <string.h>
#include <err.h>

/*
 * cat - 连接并打印文件
 * 用法: cat [文件列表]
 */

/* 打印一个已经打开的文件 */
static void docat(const char *name, int fd)
{
	char buf[1024];  // 读取缓冲区，大小1KB
	int len, wr, wrtot;

	/*
	 * 只要读取的字节数大于0，就表示还没有到达文件结尾
	 * 0表示EOF（文件结束）
	 * 小于0表示发生了错误
	 * 在某些情况下，实际读取的字节数可能少于请求的字节数
	 */
	while ((len = read(fd, buf, sizeof(buf))) > 0) {
		/*
		 * 同样地，实际写入的字节数可能少于尝试写入的字节数
		 * 所以需要循环直到所有数据都写入完成
		 */
		wrtot = 0;  // 已写入的总字节数
		while (wrtot < len) {
			// 写入剩余的数据 (buf+wrtot 指向未写入数据的起始位置)
			wr = write(STDOUT_FILENO, buf + wrtot, len - wrtot);
			if (wr < 0) {
				err(1, "stdout");  // 写入错误，退出程序
			}
			wrtot += wr;  // 更新已写入的字节数
		}
	}
	
	/*
	 * 如果读取过程中发生错误，打印错误信息并退出
	 */
	if (len < 0) {
		err(1, "%s", name);  // 显示文件名和系统错误信息
	}
}

/* 通过文件名打印文件内容 */
static
void
cat(const char *file)
{
	int fd;

	/*
	 * "-" 表示打印标准输入
	 */
	if (!strcmp(file, "-")) {
		docat("stdin", STDIN_FILENO);
		return;
	}

	/*
	 * 打开文件，打印内容，然后关闭文件
	 * 如果无法打开文件，则报错退出
	 */
	fd = open(file, O_RDONLY);  // 以只读方式打开文件
	if (fd < 0) {
		err(1, "%s", file);  // 打开文件失败，退出程序
	}
	docat(file, fd);  // 打印文件内容
	close(fd);        // 关闭文件描述符
}

int
main(int argc, char *argv[])
{
	if (argc == 1) {
		/* 没有参数 - 只处理标准输入 */
		docat("stdin", STDIN_FILENO);
	}
	else {
		/* 打印命令行中指定的所有文件 */
		int i;
		for (i = 1; i < argc; i++) {
			cat(argv[i]);  // 依次处理每个文件
		}
	}
	return 0;  // 程序正常退出
}
