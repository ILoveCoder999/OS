

#ifndef _CDEFS_H_
#define _CDEFS_H_

/*
 * 一些杂项的 C 语言定义和相关事项。
 */


/*
 * 编译时断言。不生成任何代码。失败时的错误信息不尽如人意，
 * 但鱼与熊掌不可兼得。
 */
#define COMPILE_ASSERT(x) ((void)sizeof(struct { unsigned : ((x)?1:-1); }))


/*
 * 用于静态数组元素个数的方便宏。
 */
#define ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))


/*
 * 告诉 GCC 如何检查 printf 格式。同时告知它不返回的函数，
 * 这有助于避免关于未初始化变量的错误警告。
 */
#ifdef __GNUC__
#define __PF(a,b) __attribute__((__format__(__printf__, a, b))) // printf格式检查
#define __DEAD    __attribute__((__noreturn__))                  // 函数不会返回
#define __UNUSED  __attribute__((__unused__))                    // 变量/函数可能未使用
#else
#define __PF(a,b)
#define __DEAD
#define __UNUSED
#endif


/*
 * 支持内联函数 (inline functions) 的材料。
 *
 * (此处省略了关于 C99 inline 和 GCC inline 语义差异的详细讨论，
 * 总结来说是为了解决多重定义和未定义符号的链接问题，
 * 并提供一个跨编译器版本和 C 标准的 INLINE 宏。)
 */

#if defined(__GNUC__) && !defined(__GNUC_STDC_INLINE__)
/* gcc 的非 C99 内联语义：extern inline 意味着 "不生成 out-of-line 副本" */
#define INLINE extern inline

#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
/* C99 标准：inline 意味着 "不生成 out-of-line 副本" */
#define INLINE inline

#else
/* 其他情况；static inline 是最安全的，并加上 __UNUSED 避免警告 */
#define INLINE static __UNUSED inline
#endif


#endif /* _CDEFS_H_ */
