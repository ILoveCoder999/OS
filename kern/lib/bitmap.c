/*
操作系统内核中位图（Bitmap）数据结构的 C 语言实现文件（可能是 bitmap.c）。它实现了位图的创建、销毁、资源分配和状态查询等功能
*/

/*
 * 固定大小的位数组。（ intended for storage management，用于存储管理）
 */

 #include <types.h>
 #include <kern/errno.h>
 #include <lib.h>
 #include <bitmap.h> // 包含位图的声明
 
 /*
  * 在大多数平台上，使用 uint32_t 或 unsigned long 作为存储位的基础类型会更高效。
  * 但我们没有这样做，因为如果使用任何宽度超过单个字节的数据类型，
  * 存储在磁盘上的位图数据将依赖于字节序（endian-dependent），这是一个严重的麻烦。
  */
 // ----------------------------------------------------------------------
 // 宏定义：保证跨平台和磁盘数据的字节序独立性
 // ----------------------------------------------------------------------

 /*
 在您提供的 `bitmap.c` 代码片段中，`unsigned` 和 `WORD_TYPE` 都有特定的含义：

### 1\. `unsigned` (无符号整数)

`unsigned` 是 C 语言中的一个基本类型修饰符，通常指的是 **`unsigned int`**。

  * **含义：** 它表示一个**非负整数**（即没有负数部分）。
  * **作用：** 在这个位图实现中，`unsigned` 主要用于表示**数量、大小和索引**：
      * `unsigned nbits`: 表示位图**总共管理的位数**（即资源总数）。
      * `unsigned words`: 表示位图数据存储需要多少个 `WORD_TYPE` 字节。
      * `unsigned index`: 表示在整个位图中，一个位所在的**索引号**。
      * `unsigned ix`: 表示位在 `WORD_TYPE` 数组中的**字节索引**。

### 2\. `WORD_TYPE` (位图的存储单元类型)

`WORD_TYPE` 是代码中通过 `#define` 定义的一个**宏**，用于指定位图底层数据数组的基本存储单元类型。

```c
#define WORD_TYPE       unsigned char 
#define BITS_PER_WORD   (CHAR_BIT) // CHAR_BIT 通常是 8
```

  * **含义：** 它被定义为 **`unsigned char`**（无符号字符/字节）。
  * **作用：**
    1.  **存储位数据：** `WORD_TYPE *v` 是实际存储位图数据的数组，每个元素（即一个字节）可以存储 `BITS_PER_WORD`（通常是 8）个位。
    2.  **解决字节序问题（Endianness）：** 代码注释明确提到，选择 `unsigned char`（即单字节）作为存储单元，是为了确保位图数据在存入磁盘或在不同架构的机器间传输时，**不会受到字节序（大端/小端）问题的影响**。如果使用多字节类型（如 `uint32_t`），在不同字节序的机器上读取磁盘上的位图数据时，位的顺序可能会颠倒，导致错误。

简而言之：

  * **`unsigned`**：用于**计数**和**索引**。
  * **`WORD_TYPE`**：用于实际**存储位数据**，并保证**跨平台/磁盘I/O的兼容性**。
 */
 #define BITS_PER_WORD   (CHAR_BIT) // 每个字（WORD）包含的位数，等于一个字节的位数（通常是 8）
 #define WORD_TYPE       unsigned char // 存储位的基本类型，强制使用单字节（即 char）
 #define WORD_ALLBITS    (0xff) // 一个字中所有位都设置为 1 的值 (0b11111111)
 
 // ----------------------------------------------------------------------
 // 结构体定义
 // ----------------------------------------------------------------------
 struct bitmap {
     unsigned nbits;    // 位图管理的资源总数（总位数）
     WORD_TYPE *v;      // 实际存储位的字节数组指针
 };
 
 // ----------------------------------------------------------------------
 // 函数实现：bitmap_create (创建位图)
 // ----------------------------------------------------------------------
 struct bitmap *
 bitmap_create(unsigned nbits)
 {
     struct bitmap *b;
     unsigned words; // 需要的字节（WORD）数量
 
     // 计算存储 nbits 所需的字节数，向上取整 (DIVROUNDUP(a, b) = (a+b-1)/b)
     words = DIVROUNDUP(nbits, BITS_PER_WORD); 
     
     // 1. 分配 struct bitmap 结构体本身的内存
     b = kmalloc(sizeof(struct bitmap));
     if (b == NULL) {
         return NULL; // 分配失败
     }
     
     // 2. 分配实际存储位数据的数组 v 的内存
     b->v = kmalloc(words*sizeof(WORD_TYPE));
     if (b->v == NULL) {
         kfree(b); // 失败，释放已分配的结构体内存
         return NULL;
     }
 
     // 将整个位图数组清零（0），表示所有资源初始都处于空闲状态
     bzero(b->v, words*sizeof(WORD_TYPE));
     b->nbits = nbits;
 
     /* 标记末尾任何未使用的位为“已占用”状态 */
     // 这是为了防止位图操作误触到数组末尾的、实际不对应任何资源的位。
     if (words > nbits / BITS_PER_WORD) {
         unsigned j, ix = words-1; // ix 是最后一个字节的索引
         // overbits 是最后一个字节中实际对应资源的位数 (例如 nbits=10, overbits=2)
         unsigned overbits = nbits - ix*BITS_PER_WORD; 
 
         // 确保逻辑正确性
         KASSERT(nbits / BITS_PER_WORD == words-1);
         KASSERT(overbits > 0 && overbits < BITS_PER_WORD);
 
         // 从 overbits (第一个未映射资源的位) 开始，到 BITS_PER_WORD 结束
         for (j=overbits; j<BITS_PER_WORD; j++) {
             // 将这些未映射资源的位设置为 1 (已占用)
             b->v[ix] |= ((WORD_TYPE)1 << j);
         }
     }
 
     return b;
 }
 
 // ----------------------------------------------------------------------
 // 函数实现：bitmap_getdata (获取原始数据指针)
 // ----------------------------------------------------------------------
 void *
 bitmap_getdata(struct bitmap *b)
 {
     // 返回实际存储位数据的字节数组指针，用于 I/O 操作（如读写磁盘）
     return b->v;
 }
 
 // ----------------------------------------------------------------------
 // 函数实现：bitmap_alloc (分配第一个空闲位)
 // ----------------------------------------------------------------------
 int
 bitmap_alloc(struct bitmap *b, unsigned *index)
 {
     unsigned ix; // 字节（WORD）的索引
     // 位图数组的最大索引（不包含被标记为“已占用”的溢出位）
     unsigned maxix = DIVROUNDUP(b->nbits, BITS_PER_WORD); 
     unsigned offset; // 字内的偏移量（位的索引）
 
     // 1. 遍历每个字节（WORD）
     for (ix=0; ix<maxix; ix++) {
         // 如果当前字节不全为 1 (WORD_ALLBITS)，说明里面至少有一个空闲位 (0)
         if (b->v[ix]!=WORD_ALLBITS) {
             
             // 2. 遍历当前字节内的每一位 (从 offset = 0 到 BITS_PER_WORD - 1)
             for (offset = 0; offset < BITS_PER_WORD; offset++) {
                 WORD_TYPE mask = ((WORD_TYPE)1) << offset; // 构造掩码，只有当前位是 1
 
                 // 3. 检查当前位是否为空闲 (0)
                 if ((b->v[ix] & mask)==0) {
                     
                     // 找到空闲位，将其设置为 1 (已占用)
                     b->v[ix] |= mask;
                     
                     // 计算该位在整个位图中的总索引
                     *index = (ix*BITS_PER_WORD)+offset;
                     
                     // 确保分配的索引没有超过位图的总位数
                     KASSERT(*index < b->nbits); 
                     return 0; // 成功分配
                 }
             }
             // 如果执行到这里，说明整个字节有空闲位，但在内层循环中没有找到，
             // 这与外层 if (b->v[ix]!=WORD_ALLBITS) 矛盾，应触发断言。
             KASSERT(0); 
         }
     }
     // 遍历完所有字节，没有找到空闲位，返回 ENOSPC (设备上无剩余空间) 错误
     return ENOSPC;
 }
 
 // ----------------------------------------------------------------------
 // 辅助函数：bitmap_translate (将总索引转换为字节索引和位掩码)
 // ----------------------------------------------------------------------
 static
 inline
 void
 bitmap_translate(unsigned bitno, unsigned *ix, WORD_TYPE *mask)
 {
     unsigned offset;
     // 计算位所在的字节索引
     *ix = bitno / BITS_PER_WORD; 
     // 计算位在字节内的偏移量
     offset = bitno % BITS_PER_WORD;
     // 计算用于操作该位的掩码
     *mask = ((WORD_TYPE)1) << offset;
 }
 
 // ----------------------------------------------------------------------
 // 函数实现：bitmap_mark (标记指定位为已占用)
 // ----------------------------------------------------------------------
 void
 bitmap_mark(struct bitmap *b, unsigned index)
 {
     unsigned ix;
     WORD_TYPE mask;
 
     KASSERT(index < b->nbits); // 确保索引不超过总位数
     bitmap_translate(index, &ix, &mask); // 转换索引
 
     KASSERT((b->v[ix] & mask)==0); // 确保该位在标记前是空闲的 (0)
     b->v[ix] |= mask; // 使用按位或 (OR) 将该位设置为 1
 }
 
 // ----------------------------------------------------------------------
 // 函数实现：bitmap_unmark (清除指定位为空闲)
 // ----------------------------------------------------------------------
 void
 bitmap_unmark(struct bitmap *b, unsigned index)
 {
     unsigned ix;
     WORD_TYPE mask;
 
     KASSERT(index < b->nbits); // 确保索引不超过总位数
     bitmap_translate(index, &ix, &mask); // 转换索引
 
     KASSERT((b->v[ix] & mask)!=0); // 确保该位在清除前是已占用的 (1)
     b->v[ix] &= ~mask; // 使用按位与 (AND) 和反转掩码 (~) 将该位设置为 0
 }
 
 
 // ----------------------------------------------------------------------
 // 函数实现：bitmap_isset (检查指定位状态)
 // ----------------------------------------------------------------------
 int
 bitmap_isset(struct bitmap *b, unsigned index)
 {
     unsigned ix;
     WORD_TYPE mask;
 
     // KASSERT(index < b->nbits); // 实际上这里应该有 KASSERT，但代码中没有
     bitmap_translate(index, &ix, &mask); // 转换索引
     
     // 返回 (b->v[ix] & mask) 的结果。如果该位为 1，结果非零 (true)；否则为零 (false)。
     return (b->v[ix] & mask); 
 }
 
 // ----------------------------------------------------------------------
 // 函数实现：bitmap_destroy (销毁位图)
 // ----------------------------------------------------------------------
 void
 bitmap_destroy(struct bitmap *b)
 {
     kfree(b->v); // 释放实际存储位数据的数组内存
     kfree(b);   // 释放 struct bitmap 结构体本身的内存
 }

 /*
 您的问题是关于 **Linux 是否通过位图来管理存储**。

答案是：**Linux 在其存储管理的不同层面会使用位图（Bitmap）技术，但位图不是唯一的或主要的管理方式，它通常是作为辅助结构存在的。**

### 1. 文件系统层面 (Filesystem)

在 Linux 的文件系统（如 ext4、Btrfs）中，位图是**管理块分配**的一种常见且高效的机制：

* **ext4/ext3 (最常见的 Linux 文件系统)：** ext4 使用**块组（Block Group）**来组织磁盘空间。
    * 每个块组中都包含一个 **块位图 (Block Bitmap)**，用来跟踪该块组内**哪些数据块是空闲的**（0 表示空闲，1 表示已占用）。
    * 它还有一个 **i 节点位图 (Inode Bitmap)**，用于跟踪该块组内**哪些 i 节点（索引节点）是空闲的**。

* **优点：** 使用位图可以**快速**找到一个空闲块或 i 节点，而且这种表示方式**空间效率很高**（每个块只需要一个位）。

### 2. 内存管理层面 (Memory Management)

在管理物理内存时，位图也扮演着重要角色：

* **伙伴系统 (Buddy System)：** 这是 Linux 管理物理内存页分配的核心算法。虽然伙伴系统主要依赖链表结构来管理不同大小的空闲块，但在某些实现中或对于特定的内存池，可能会使用位图来快速跟踪**最小分配单元（通常是 4KB 页面）**的空闲状态。

### 3. 逻辑卷管理层面 (LVM)

* **LVM (Logical Volume Management)** 可能会使用位图来跟踪逻辑卷的**数据同步状态**或**分配的扩展块 (Extents)**，尤其是在进行快照（Snapshots）或镜像（Mirrors）操作时。

### 总结

虽然 Linux 的存储管理是一个复杂的系统，涉及多种数据结构（如 B 树、链表、平衡树等），但**位图因其高效率和低空间消耗**的特点，被广泛应用于**文件系统的块和 i 节点分配管理**中。

您之前提供的 os161 代码中的 `bitmap.h` 就是这种思想的体现，它为 os161 内核提供了管理资源的通用位图工具。
 */