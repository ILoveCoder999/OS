
#ifndef _BITMAP_H_
#define _BITMAP_H_

/*
 * 固定大小的位数组。（旨在用于存储管理。）
 *
 * 函数:
 * bitmap_create  - 分配一个新的位图对象。
 * 出错时返回 NULL。
 * bitmap_getdata - 返回原始位数据的指针（用于 I/O）。
 * bitmap_alloc   - 找到一个清零的位，将其置位，并返回其索引。
 * bitmap_mark    - 通过索引设置一个清零的位。
 * bitmap_unmark  - 通过索引清除一个已设置的位。
 * bitmap_isset   - 返回一个特定位是否已设置。
 * bitmap_destroy - 销毁位图。
 */


struct bitmap;  /* 不透明结构体 (Opaque)。实际定义在对应的 .c 文件中。 */

struct bitmap *bitmap_create(unsigned nbits);
void          *bitmap_getdata(struct bitmap *);
int            bitmap_alloc(struct bitmap *, unsigned *index);
void           bitmap_mark(struct bitmap *, unsigned index);
void           bitmap_unmark(struct bitmap *, unsigned index);
int            bitmap_isset(struct bitmap *, unsigned index);
void           bitmap_destroy(struct bitmap *);


#endif /* _BITMAP_H_ */
