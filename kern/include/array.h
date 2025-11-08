
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <cdefs.h> // 包含 C 语言定义，如 INLINE
#include <lib.h>   // 包含库函数，如 kmalloc/kfree

#define ARRAYS_CHECKED // 启用数组边界检查

#ifdef ARRAYS_CHECKED
#define ARRAYASSERT KASSERT // 使用内核断言 KASSERT 进行检查
#else
#define ARRAYASSERT(x) ((void)(x)) // 禁用检查
#endif

#ifndef ARRAYINLINE
#define ARRAYINLINE INLINE // 默认使用 cdefs.h 中定义的 INLINE
#endif

/*
 * 基础数组类型（可变大小的 void 指针数组）及操作。
 *
 * create - 分配一个数组。
 * destroy - 销毁一个已分配的数组。
 * init - 在外部已分配的空间中初始化一个数组。
 * cleanup - 清理在外部已分配空间中的数组。
 * num - 返回数组中的元素数量。
 * get - 返回索引号为 INDEX 的元素。
 * set - 将索引号为 INDEX 的元素设置为 VAL。
 * preallocate - 预分配空间但不改变大小；可能失败并返回错误。
 * setsize - 改变大小为 NUM 个元素；可能失败并返回错误。
 * add - 将 VAL 追加到数组末尾；如果 INDEX_RET 不为 NULL，则在其中返回其索引；
 * 可能失败并返回错误。
 * remove - 删除索引 INDEX 处的条目，并将后续条目向前移动以填补空隙。
 *
 * 注意：使用 setsize 扩展数组时，不会初始化新的元素。
 * （通常调用者会立即向它们存储数据。）
 */

struct array {
	void **v;      // 存储数据的指针数组
	unsigned num, max; // 当前元素数量和最大容量
};

struct array *array_create(void);
void array_destroy(struct array *);
void array_init(struct array *);
void array_cleanup(struct array *);
ARRAYINLINE unsigned array_num(const struct array *);
ARRAYINLINE void *array_get(const struct array *, unsigned index);
ARRAYINLINE void array_set(const struct array *, unsigned index, void *val);
int array_preallocate(struct array *, unsigned num);
int array_setsize(struct array *, unsigned num);
ARRAYINLINE int array_add(struct array *, void *val, unsigned *index_ret);
void array_remove(struct array *, unsigned index);

/*
 * 基础操作的内联实现
 */

// ... (以下是 array_num, array_get, array_set, array_add 的内联定义) ...
// (内联函数主体：检查、操作结构体成员并调用 setsize/get/set)

ARRAYINLINE unsigned
array_num(const struct array *a)
{
	return a->num;
}

ARRAYINLINE void *
array_get(const struct array *a, unsigned index)
{
	ARRAYASSERT(index < a->num);
	return a->v[index];
}

ARRAYINLINE void
array_set(const struct array *a, unsigned index, void *val)
{
	ARRAYASSERT(index < a->num);
	a->v[index] = val;
}

ARRAYINLINE int
array_add(struct array *a, void *val, unsigned *index_ret)
{
	unsigned index;
	int ret;

	index = a->num;
	ret = array_setsize(a, index+1);
	if (ret) {
		return ret;
	}
	a->v[index] = val;
	if (index_ret != NULL) {
		*index_ret = index;
	}
	return 0;
}


/*
 * 用于声明和定义类型化数组的宏。
 *
 * 用法:
 *
 * DECLARRAY_BYTYPE(foo, bar) 声明 "struct foo"，它是一个指向 "bar" 的指针数组，
 * 以及对它的操作。
 *
 * DECLARRAY(foo) 等同于 DECLARRAY_BYTYPE(fooarray, struct foo)。
 *
 * DEFARRAY_BYTYPE 和 DEFARRAY 与 DECLARRAY 相同，但它们定义了操作，
 * 并且都接受一个额外的参数 INLINE。对于 C99，这应该在头文件中是 INLINE，
 * 在主源文件中是空（empty），与 ARRAYINLINE 在 array.c 中的用法相同。
 *
 * (省略了宏 DECLARRAY_BYTYPE, DEFARRAY_BYTYPE, DECLARRAY, DEFARRAY 的详细定义)
 * 这些宏创建了类型化的数组结构体和相应的操作函数（如 foo_create, foo_num, foo_get等）。
 */

#define DECLARRAY_BYTYPE(ARRAY, T, INLINE) \
	/* 声明 struct ARRAY, 函数原型 */
    /* ... 声明了 ARRAY##_create, ARRAY##_destroy, ARRAY##_num, ARRAY##_get 等函数原型 ... */

#define DEFARRAY_BYTYPE(ARRAY, T, INLINE) \
	/* 定义 struct ARRAY 的操作函数 */
    /* ... 实现了 ARRAY##_create, ARRAY##_destroy, ARRAY##_num, ARRAY##_get 等函数 ... */

#define DECLARRAY(T, INLINE) DECLARRAY_BYTYPE(T##array, struct T, INLINE)
#define DEFARRAY(T, INLINE) DEFARRAY_BYTYPE(T##array, struct T, INLINE)

/*
 * 这是声明一个字符串数组的方式；它被解析为指向 char 的指针数组。
 */
DECLARRAY_BYTYPE(stringarray, char, ARRAYINLINE);
DEFARRAY_BYTYPE(stringarray, char, ARRAYINLINE);


#endif /* ARRAY_H */
