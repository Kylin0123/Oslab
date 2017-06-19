#ifndef __X86_MEMORY_H__
#define __X86_MEMORY_H__

#define DPL_KERN                0
#define DPL_USER                3

// Application segment type bits
#define STA_X       0x8         // Executable segment
#define STA_W       0x2         // Writeable (non-executable segments)
#define STA_R       0x2         // Readable (executable segments)

// System segment type bits
#define STS_T32A    0x9         // Available 32-bit TSS
#define STS_IG32    0xE         // 32-bit Interrupt Gate
#define STS_TG32    0xF         // 32-bit Trap Gate

// GDT entries
#define NR_SEGMENTS      7           // GDT size
#define SEG_KCODE   1           // Kernel code
#define SEG_KDATA   2           // Kernel data/stack
#define SEG_UCODE   3           // User code
#define SEG_UDATA   4           // User data/stack
#define SEG_TSS     5           // Global unique task state segement
#define SEG_VIDEO   6

// Selectors
#define KSEL(desc) (((desc) << 3) | DPL_KERN)
#define USEL(desc) (((desc) << 3) | DPL_USER)

struct GateDescriptor {
	uint32_t offset_15_0      : 16;
	uint32_t segment          : 16;
	uint32_t pad0             : 8;
	uint32_t type             : 4;
	uint32_t system           : 1;
	uint32_t privilege_level  : 2;
	uint32_t present          : 1;
	uint32_t offset_31_16     : 16;
};

//struct TrapFrame {
//	uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
//	int32_t irq;
//};

struct TrapFrame {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
    uint32_t irq;                   // 中断号
    uint32_t error;                 // Error Code
    uint32_t eip, cs, eflags, esp, ss;
};

//TODO: PCB implementation
#define RUNNING     0
#define RUNNABLE    1
#define BLOCKED     2
#define DEAD        3

#define MAX_PCB_NUM     512
#define MAX_STACK_SIZE  8192

struct ProcessTable {
    uint32_t    stack[MAX_STACK_SIZE]; // 内核堆栈
    struct TrapFrame    tf;
    int         state;
    int         timeCount;
    int         sleepTime;
    uint32_t    pid;
    int         valid;
};

struct semaphore{
    int value;
    struct ProcessTable * list;
};

/*
1. The number of bits in a bit field sets the limit to the range of values it can hold
2. Multiple adjacent bit fields are usually packed together (although this behavior is implementation-defined)

Refer: en.cppreference.com/w/cpp/language/bit_field
*/
struct SegDesc {
	uint32_t lim_15_0 : 16;  // Low bits of segment limit
	uint32_t base_15_0 : 16; // Low bits of segment base address
	uint32_t base_23_16 : 8; // Middle bits of segment base address
	uint32_t type : 4;       // Segment type (see STS_ constants)
	uint32_t s : 1;          // 0 = system, 1 = application
	uint32_t dpl : 2;        // Descriptor Privilege Level
	uint32_t p : 1;          // Present
	uint32_t lim_19_16 : 4;  // High bits of segment limit
	uint32_t avl : 1;        // Unused (available for software use)
	uint32_t rsv1 : 1;       // Reserved
	uint32_t db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
	uint32_t g : 1;          // Granularity: limit scaled by 4K when set
	uint32_t base_31_24 : 8; // High bits of segment base address
};
typedef struct SegDesc SegDesc;

#define SEG(type, base, lim, dpl) (SegDesc)                   \
{	((lim) >> 12) & 0xffff, (uint32_t)(base) & 0xffff,        \
	((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,         \
	(uint32_t)(lim) >> 28, 0, 0, 1, 1, (uint32_t)(base) >> 24 }

#define SEG16(type, base, lim, dpl) (SegDesc)                 \
{	(lim) & 0xffff, (uint32_t)(base) & 0xffff,                \
	((uint32_t)(base) >> 16) & 0xff, type, 0, dpl, 1,         \
	(uint32_t)(lim) >> 16, 0, 0, 1, 0, (uint32_t)(base) >> 24 }
	
// Task state segment format
struct TSS {
	uint32_t link;         // old ts selector
	uint32_t esp0;         // Ring 0 Stack pointer and segment selector
	uint32_t ss0;          // after an increase in privilege level
	union{
		struct{
			char dontcare[88];
		};
		struct{
			uint32_t esp1,ss1,esp2,ss2;
			uint32_t cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
			uint32_t es, cs, ss, ds, fs, gs, ldt;
		};
        };
};
typedef struct TSS TSS;

static inline void setGdt(SegDesc *gdt, uint32_t size) {
	volatile static uint16_t data[3];
	data[0] = size - 1;
	data[1] = (uint32_t)gdt;
	data[2] = (uint32_t)gdt >> 16;
	asm volatile("lgdt (%0)" : : "r"(data));
}

static inline void lLdt(uint16_t sel)
{
	asm volatile("lldt %0" :: "r"(sel));
}

/*File system structures*/
#define POINTER_NUM     10
#define NAME_LENGTH     10
#define SUPER_BLOCK_SIZE    1024
#define GROUP_DESC_SIZE     1024
#define INODE_SIZE      1024
#define DIRENTRY_SIZE   (4+NAME_LENGTH)
union SuperBlock {
      uint8_t byte[SUPER_BLOCK_SIZE];
      struct {
             int32_t sectorNum;                   // 文件系统中扇区总数
             int32_t inodeNum;                    // 文件系统中inode总数
             int32_t blockNum;                    // 文件系统中data block总数
             int32_t availInodeNum;               // 文件系统中可用inode总数
             int32_t availBlockNum;               // 文件系统中可用data block总数
             int32_t blockSize;                   // 每个block所含字节数
             int32_t inodesPerGroup;              // 每个group所含inode数
             int32_t blocksPerGroup;              // 每个group所含data block数
      };
};

union GroupDesc {                                 // Group Descriptor Table的表项
      uint8_t byte[GROUP_DESC_SIZE];
      struct {
             int32_t inodeBitmap;                 // 该group中inodeBitmap的偏移量
             int32_t blockBitmap;                 // 该group中blockBitmap的偏移量
             int32_t inodeTable;                  // 该group中inodeTable的偏移量
             int32_t availInodeNum;               // 该group中可用inode总数
             int32_t availBlockNum;               // 该group中可用data block总数
      };
};

union Inode {                                     // Inode Table的表项
      uint8_t byte[INODE_SIZE];
      struct {
             int16_t type;                        // 该文件的类型、访存控制等
             int16_t linkCount;                   // 该文件的链接数
             int32_t blockCount;                  // 该文件的data block总数
             int32_t size;                        // 该文件所含字节数
             int32_t pointer[POINTER_NUM];        // data block偏移量
             int32_t singlyPointer;               // 一级data block偏移量索引
             int32_t doublyPointer;               // 二级data block偏移量索引
             int32_t triplyPointer;               // 三级data block偏移量索引
      };
};

union DirEntry {                                  // 目录文件的表项
      uint8_t byte[DIRENTRY_SIZE];
      struct {
             int32_t inode;                       // 该目录项对应的inode的偏移量
             char name[NAME_LENGTH];              // 该目录项对应的文件名
      };
};

union DirEntryTable {
    uint8_t byte[1024];
    struct{
        uint32_t size;
        union DirEntry dirs[3];
    };
};

#endif
