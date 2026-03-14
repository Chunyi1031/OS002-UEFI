/*
    Interrupt.h IDT.h PIC.h IDT.c PIC.c
 中断系统库
-  中断处理

2026/3/6 Liu Chunyi

*/

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <Tools.h>
#include <interrupt/IDT.h>//中断描述符库
#include <interrupt/PIC.h>

// 异常向量号宏定义（0-31）
#define IDT_DE_VECTOR   0   // 除法错误 (#DE)
#define IDT_DB_VECTOR   1   // 调试异常 (#DB)
#define IDT_NMI_VECTOR  2   // 非屏蔽中断 (NMI)
#define IDT_BP_VECTOR   3   // 断点 (#BP)
#define IDT_OF_VECTOR   4   // 溢出 (#OF)
#define IDT_BR_VECTOR   5   // 边界范围 (#BR)
#define IDT_UD_VECTOR   6   // 无效操作码 (#UD)
#define IDT_NM_VECTOR   7   // 设备不可用 (#NM)
#define IDT_DF_VECTOR   8   // 双重故障 (#DF)
// 向量 9 保留（协处理器段超限，386后不再使用）
#define IDT_TS_VECTOR   10  // 无效 TSS (#TS)
#define IDT_NP_VECTOR   11  // 段不存在 (#NP)
#define IDT_SS_VECTOR   12  // 堆栈段错误 (#SS)
#define IDT_GP_VECTOR   13  // 通用保护故障 (#GP)
#define IDT_PF_VECTOR   14  // 页故障 (#PF)
// 向量 15 保留
#define IDT_MF_VECTOR   16  // x87 浮点错误 (#MF)
#define IDT_AC_VECTOR   17  // 对齐检查 (#AC)
#define IDT_MC_VECTOR   18  // 机器检查 (#MC)
#define IDT_XM_VECTOR   19  // SIMD 浮点异常 (#XM)
#define IDT_VE_VECTOR   20  // 虚拟化异常 (#VE)
// 向量 21–31 保留

#define PIT_CLOCK_FREQ 500

#endif