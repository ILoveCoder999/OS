
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
/*
os161 内核中使用的位图（Bitmap）数据结构及其操作函数。
位图是一种固定大小的位（bit）数组，常用于操作系统内核中进行存储管理和资源分配，因为它能以最小的内存开销表示大量资源的占用状态.
存储管理
这个位图的主要设计目的是用于存储管理。在 os161 或任何操作系统内核中，位图通常用来跟踪资源是否被占用，例如：
物理内存管理： 跟踪每个**内存页（page）**是否被分配。
磁盘空间管理： 跟踪每个**磁盘块（block）**是否被文件系统占用。
其他资源分配： 跟踪文件描述符、进程 ID 等是否被使用。
核心思想：
1 代表该资源（位图中的位索引对应的资源）已占用/已设置。
0 代表该资源空闲/未设置。

*/
/*
文件中的 struct bitmap; 
是一个**不透明结构体（Opaque Structure）**的声明，
这意味着它的具体内容在 .h 文件中是隐藏的，用户只能通过提供的函数来操作它。
允许内核快速地分配、释放和检查数百万个资源的状态，而只占用极少的内存空间
*/

struct bitmap;  /* 不透明结构体 (Opaque)。实际定义在对应的 .c 文件中。 */

/*
bitmap_create(unsigned nbits)	创建/分配	
分配并初始化一个新的位图对象，用于管理 nbits 数量的资源。
如果分配失败，返回 NULL。	内核启动时初始化内存管理器或文件系统时调用。
*/
struct bitmap *bitmap_create(unsigned nbits);
/*
bitmap_create(unsigned nbits)	创建/分配	
分配并初始化一个新的位图对象，用于管理 nbits 数量的资源。
如果分配失败，返回 NULL。	内核启动时初始化内存管理器或文件系统时调用。
*/
void          *bitmap_getdata(struct bitmap *);
/*bitmap_alloc(struct bitmap *, unsigned *index)	分配资源	
定位并返回第一个未设置（清空，即值为 0）的位。
找到后，会自动将其设置为已设置（mark/1），
并将该位的索引通过 index 参数返回。	
查找一个空闲内存页或空闲磁盘块。

*/
int            bitmap_alloc(struct bitmap *, unsigned *index);
/*
bitmap_mark(struct bitmap *, unsigned index)	设置/占用	
设置（mark，置为 1）索引 index 处的位。这通常在资源被手动分配后使用。	
将某个已知的空闲位标记为已占用
*/
void           bitmap_mark(struct bitmap *, unsigned index);
/*
bitmap_mark(struct bitmap *, unsigned index)	设置/占用	
设置（mark，置为 1）索引 index 处的位。
这通常在资源被手动分配后使用。	将某个已知的空闲位标记为已占用
*/
void           bitmap_unmark(struct bitmap *, unsigned index);
bitmap_isset(struct bitmap *, unsigned index)	检查状态	
检查索引 index 处的位是否已设置（返回非零表示已设置，即已占用）。	
在使用资源前检查它是否真的被占用。
int            bitmap_isset(struct bitmap *, unsigned index);
/*
bitmap_destroy(struct bitmap *)	销毁/释放	
销毁位图对象，并释放其占用的内存。	
在系统关闭或卸载某个组件时回收资源。
*/
void           bitmap_destroy(struct bitmap *);


#endif /* _BITMAP_H_ */
