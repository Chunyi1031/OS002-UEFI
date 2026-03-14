/*
    Memory.h MemoryDescriptor.c PMemory.c Bitmap.c
 内存管理库
- UEFI内存描述符识别
- 物理内存管理
- 位图结构

2026/3/6 Liu Chunyi

*/

#ifndef MEMORY_H
#define MEMORY_H

#include <Tools.h>
#include <Print.h>

// UEFI 规范定义的内存属性位
#define MEMORY_UC         0x0000000000000001  // Uncached - 不可缓存
#define MEMORY_WC         0x0000000000000002  // Write Combining - 写组合
#define MEMORY_WT         0x0000000000000004  // Write Through - 写通过
#define MEMORY_WB         0x0000000000000008  // Write Back - 写回
#define MEMORY_UCE        0x0000000000000010  // Uncached Export - 不可缓存导出
#define MEMORY_WP         0x0000000000001000  // Write Protect - 写保护
#define MEMORY_RP         0x0000000000002000  // Read Protect - 读保护
#define MEMORY_XP         0x0000000000004000  // Execute Protect - 执行保护
#define MEMORY_RO         0x0000000000020000  // Read Only -

//内存类型定义
#define RESERVED_MEMORY_TYPE        0
#define LOADER_CODE                 1
#define LOADER_DATA                 2
#define BOOT_SERVICES_CODE          3
#define BOOT_SERVICES_DATA          4
#define RUNTIME_SERVICES_CODE       5
#define RUNTIME_SERVICES_DATA       6
#define CONVENTIONAL_MEMORY         7   //用内存
#define UNUSABLE_MEMORY             8   //不可用
#define ACPI_RECLAIM_MEMORY         9   //可用但会被ACPI回收
#define ACPI_MEMORY_NVS             10  //不可用
#define MEMORY_MAPPED_IO            11  //不可用（MMIO）
#define MEMORY_MAPPED_IO_PORT_SPACE 12  //不可用
#define PAL_CODE                    13  //不可用（安腾处理器）
#define PERSISTENT_MEMORY           14  //持久内存

// 定义内存描述符结构体，用于描述一段内存区域的信息
typedef struct MEMORY_DESCRIPTOR{
    uint32_t        Type;             //内存类型（如可用内存、保留内存等）
    uint32_t        Pad;              //填充字段，用于对齐
    uintptr_t       PhysicalStart;    //物理起始地址
    uintptr_t       VirtualStart;     //虚拟起始地址
    uint64_t        NumberOfPages;    //页数量
    uint64_t        Attribute;        //属性（如可读、可写、可执行等）
} MEMORY_DESCRIPTOR;

extern MEMORY_MAP* MemoryMap;//全局内存映射

//内存描述符(MemoryDescriptor)函数定义
STATUS CheckMemoryMap(MEMORY_MAP* MemoryMap);//检查内存映射的有效性
void PrintMemoryAttributes(uint64_t Attribute);//打印内存属性
MEMORY_DESCRIPTOR* AnalysisMemoryMap(MEMORY_MAP* MemoryMap,const uint32_t num);//解析某个内存描述符的信息
void PrintMemoryDescriptor(MEMORY_DESCRIPTOR* DescriptorAddr);//打印某个内存描述符的信息
STATUS IsMemoryAvailable(MEMORY_DESCRIPTOR *Desc);//检查该描述符的内存是否可用
void PrintAvailableMemoryDescriptor(MEMORY_MAP* MemoryMap,const uint32_t num);//打印可用指定数量的内存的编号
MEMORY_DESCRIPTOR* FindLargestAvailableBlock(MEMORY_MAP* MemoryMap);//查找最大的可用内存块
uint64_t GetMemoryTotalSize(MEMORY_MAP* MemoryMap);//获取总内存大小
void* AllocatePage(uint64_t size);//页级分配内存
void FreePage(void* addr);//页级回收内存

//物理内存管理(PMemory)函数定义
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
typedef struct OS_MEMORY_DESCRIPTOR{
    uint32_t Type;
    uint64_t PhysicalStart;
    uint64_t PageSize;
    uint8_t* bitmap;
}OS_MEMORY_DESCRIPTOR;
extern OS_MEMORY_DESCRIPTOR* gOSMemoryDescriptors;//全局OS内存描述符数组
extern uint32_t gOSMemDescCount;//全局OS内存描述符数量
STATUS InitPhysicalMemoryManager(MEMORY_MAP* MemoryMap);//物理内存管理器初始化
void* pmm_alloc_pages(size_t count);//分配指定数量的物理页面
void pmm_free_pages(void* addr, size_t count);//释放指定数量的物理页面

#endif