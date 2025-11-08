

#ifndef _DEVICE_H_
#define _DEVICE_H_

/*
 * 设备。
 */


struct uio;  /* 在 <uio.h> 中定义 (用于 I/O 描述符) */

/*
 * 文件系统命名空间可访问的设备。
 */
struct device {
	const struct device_ops *d_ops; // 设备操作函数集

	blkcnt_t d_blocks;              // 块设备的总块数
	blksize_t d_blocksize;          // 块设备的块大小

	dev_t d_devnumber;	/* 此设备的序列号 (设备号) */

	void *d_data;		/* 设备特定的数据 */
};

/*
 * 设备操作 (Device operations)。
 * devop_eachopen - 在每次 open 调用时被调用，允许拒绝打开操作
 * devop_io - 用于读取和写入（uio 指示方向）
 * devop_ioctl - 杂项控制操作
 */
struct device_ops {
	int (*devop_eachopen)(struct device *, int flags_from_open);
	int (*devop_io)(struct device *, struct uio *);
	int (*devop_ioctl)(struct device *, int op, userptr_t data);
};

/*
 * 简化调用序列的宏。
 */
#define DEVOP_EACHOPEN(d, f)	((d)->d_ops->devop_eachopen(d, f))
#define DEVOP_IO(d, u)		((d)->d_ops->devop_io(d, u))
#define DEVOP_IOCTL(d, op, p)	((d)->d_ops->devop_ioctl(d, op, p))


/* 为 VFS 级别的设备创建 vnode (虚拟节点)。 */
struct vnode *dev_create_vnode(struct device *dev);

/* 撤销 dev_create_vnode 的操作。 */
void dev_uncreate_vnode(struct vnode *vn);

/* 内置 VFS 级别设备的初始化函数。 */
void devnull_create(void); // 创建 /dev/null 设备

/* 启动设备探测和附加（attach）的函数。 */
void dev_bootstrap(void);


#endif /* _DEVICE_H_ */
