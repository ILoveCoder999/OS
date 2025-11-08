

#ifndef _ELF_H_
#define _ELF_H_


/*
 *
 * 限制:
 * 仅支持 32 位
 * 不支持 .o 文件或链接器结构
 * 未定义标准 elf 头文件中的所有随机符号。
 */

/* 获取机器相关位 */
#include <machine/elf.h>


/*
 * ELF 文件头 (File header)。它出现在 ELF 文件的最开始。
 */
#define	ELF_NIDENT	16
typedef struct {
	unsigned char	e_ident[ELF_NIDENT];   /* 魔数等标识 */
	uint16_t	e_type;                /* 文件类型 */
	uint16_t	e_machine;             /* 文件所针对的处理器类型 */
	uint32_t	e_version;             /* ELF 版本 */
	uint32_t	e_entry;           /* 程序入口点的地址 */
	uint32_t	e_phoff;           /* 文件中程序头表 (phdr) 的位置 */
	uint32_t	e_shoff;           /* 忽略 (节头表位置) */
	uint32_t	e_flags;	   /* 忽略 */
	uint16_t	e_ehsize;          /* 文件头实际大小 */
	uint16_t	e_phentsize;       /* 程序头 (phdr) 实际大小 */
	uint16_t	e_phnum;           /* 程序头数量 */
	uint16_t	e_shentsize;       /* 忽略 (节头大小) */
	uint16_t	e_shnum;           /* 忽略 (节头数量) */
	uint16_t	e_shstrndx;        /* 忽略 (节名称字符串表索引) */
} Elf32_Ehdr;

/* e_ident[] 中 1 字节字段的偏移量 */
// ... (定义了 EI_MAG0 到 EI_PAD 的偏移量宏) ...

/* 这些字段的值 */

/* 对于 e_ident[EI_MAG0..3] (ELF 魔数) */
// ... (定义了 ELFMAG0 到 ELFMAG3 的值宏) ...

/* 对于 e_ident[EI_CLASS] (文件类别) */
#define	ELFCLASSNONE	0	/* 无效类别 */
#define	ELFCLASS32	1	/* 32 位对象 */
#define	ELFCLASS64	2	/* 64 位对象 */

/* e_ident[EI_DATA] (数据编码/字节序) */
#define	ELFDATANONE	0	/* 无效数据编码 */
#define	ELFDATA2LSB	1	/* 2's complement, LSB first (小端) */
#define	ELFDATA2MSB	2	/* 2's complement, MSB first (大端) */

/* e_ident[EI_VERSION] (ELF 版本) */
#define	EV_NONE		0	/* 无效版本 */
#define	EV_CURRENT	1	/* 当前版本 */

/* e_ident[EI_OSABI] (操作系统/syscall ABI 标识) */
// ... (定义了 ELFOSABI_SYSV, ELFOSABI_HPUX, ELFOSABI_STANDALONE 的值宏) ...


/*
 * e_type 的值 (文件类型)
 */
#define	ET_NONE		0	/* 无文件类型 */
#define	ET_REL		1	/* 可重定位文件 */
#define	ET_EXEC		2	/* 可执行文件 */
#define	ET_DYN		3	/* 共享对象文件 */
#define	ET_CORE		4	/* 核心转储文件 */
// ...

/*
 * e_machine 的值 (处理器类型)
 */
// ... (定义了 EM_NONE, EM_MIPS 等处理器类型的值宏) ...


/*
 * "程序头 (Program Header)" - 运行时段头。
 * 文件中有一个位置存放着 Ehdr.e_phnum 个这种结构体。
 *
 * 注意: 如果 p_memsz > p_filesz，剩余空间应该被零填充。
 */
typedef struct {
	uint32_t	p_type;      /* 段类型 */
	uint32_t	p_offset;    /* 文件中数据的位置 */
	uint32_t	p_vaddr;     /* 虚拟地址 */
	uint32_t	p_paddr;     /* 忽略 (物理地址) */
	uint32_t	p_filesz;    /* 文件中数据的大小 */
	uint32_t	p_memsz;     /* 加载到内存中的数据大小 */
	uint32_t	p_flags;     /* 标志 */
	uint32_t	p_align;     /* 所需对齐 - 可以忽略 */
} Elf32_Phdr;

/* p_type 的值 (段类型) */
#define	PT_NULL		0		/* 程序头表条目未使用 */
#define	PT_LOAD		1		/* 可加载程序段 */
// ... (定义了 PT_DYNAMIC, PT_INTERP 等其他段类型的值宏) ...

/* p_flags 的值 (权限标志) */
#define	PF_R		0x4	/* 段是可读的 */
#define	PF_W		0x2	/* 段是可写的 */
#define	PF_X		0x1	/* 段是可执行的 */


typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Phdr Elf_Phdr;


#endif /* _ELF_H_ */
