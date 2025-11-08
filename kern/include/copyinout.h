#kernel/userland data copy

#ifndef _COPYINOUT_H_
#define _COPYINOUT_H_


/*
 * copyin/copyout/copyinstr/copyoutstr 是标准的 BSD 内核函数。
 *
 * copyin 从用户空间地址 USERSRC 复制 LEN 字节到内核空间地址 DEST。
 *
 * copyout 从内核空间地址 SRC 复制 LEN 字节到用户空间地址 USERDEST。
 *
 * copyinstr 从用户空间地址 USERSRC 复制一个最多 LEN 字节的空终止字符串到
 * 内核空间地址 DEST，并在 GOT 中返回找到的实际字符串长度。成功时 DEST 总是空终止的。
 * LEN 和 GOT 都包含空终止符。
 *
 * copyoutstr 从内核空间地址 SRC 复制一个最多 LEN 字节的空终止字符串到
 * 用户空间地址 USERDEST，并在 GOT 中返回找到的实际字符串长度。成功时
 * USERDEST 总是空终止的。LEN 和 GOT 都包含空终止符。
 *
 * 所有这些函数：
 * - 成功时返回 0。
 * - 如果遇到内存寻址错误，返回 EFAULT。
 * - （对于字符串版本）如果可用空间不足，返回 ENAMETOOLONG。
 *
 * 注意：参数的顺序与 bcopy() 或 cp/mv 相同，即**源地址在左**，与 strcpy() **不同**。
 * const 限定符和类型有助于防止这方面的错误，但显然不是万无一失的。
 *
 * 这些函数是机器依赖的；然而，一个可被多种机器类型使用的通用版本
 * 位于 vm/copyinout.c 中。
 */

int copyin(const_userptr_t usersrc, void *dest, size_t len);
int copyout(const void *src, userptr_t userdest, size_t len);
int copyinstr(const_userptr_t usersrc, char *dest, size_t len, size_t *got);
int copyoutstr(const char *src, userptr_t userdest, size_t len, size_t *got);


#endif /* _COPYINOUT_H_ */
