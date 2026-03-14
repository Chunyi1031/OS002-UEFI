/* 内核链接脚本 */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

/* 内核加载地址 */
KERNEL_VMA = 0xffffffff80000000;  /* 高半内核地址 */
KERNEL_LMA = 0x10000;            /* 1MB物理地址 */

SECTIONS
{
    /* 从虚拟地址开始，但加载到物理地址 */
    . = KERNEL_VMA;
    
    __kernel_start = .;
    
    /* 文本段 - 必须是第一个段 */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA)
    {
        /* 确保_start函数在最前面 */
        *(.text.start)
        KEEP(*(.text.start))
        
        /* 其他代码 */
        *(.text)
        *(.text.*)
        
        /* 确保_stop函数保留 */
        KEEP(*(.text.stop))
    }
    
    /* 只读数据 */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* 数据段 */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.data)
        *(.data.*)
    }
    
    /* 未初始化数据 (BSS) */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA)
    {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }
    
    /* 结束标记 */
    __kernel_end = .;
    
    /* 丢弃不需要的段 */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}