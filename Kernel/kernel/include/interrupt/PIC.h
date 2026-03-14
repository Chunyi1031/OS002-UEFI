/*
中断控制器库
*/

#ifndef PIC_H
#define PIC_H

#include <Interrupt.h>
#include <interrupt/IDT.h>
#include <Tools.h>

#define ICW_1 0x11

#define PIC_1_CTRL 0x20 //主PIC控制器端口
#define PIC_2_CTRL 0xA0 //从PIC控制器端口
#define PIC_1_DATA 0x21 //主PIC数据端口
#define PIC_2_DATA 0xA1 //从PIC数据端口

#define IRQ_0 0x20//IRQs 0-7 使用中断0x20-0x27
#define IRQ_8 0x28//IRQs 8-15 使用中断0x28-0x36

#define PIC_EOI		0x20

#define PIC0_ICW1 0x20
#define PIC0_ICW2 0x21
#define PIC0_ICW3 0x21
#define PIC0_ICW4 0x21
#define PIC0_OCW2 0x20
#define PIC0_IMR  0x21

#define PIC1_ICW1 0xA0
#define PIC1_ICW2 0xA1
#define PIC1_ICW3 0xA1
#define PIC1_ICW4 0xA1
#define PIC1_OCW2 0xA0
#define PIC1_IMR  0xA1

#define PIC_ICW1_ALWAYS_1   (1 << 4)
#define PIC_ICW1_ICW4       (1 << 0)
#define PIC_ICW4_8086       (1 << 0)
#define PIC_OCW2_EOI        (1 << 5)
#define IRQ_PIC_START       0x20

// IRQ 编号（0-15），用于PIC_sendEOI等函数
#define IRQ0      0   // 定时器
#define IRQ1      1   // 键盘
#define IRQ2      2   // 级联从片（不可用）
#define IRQ3      3   // COM2
#define IRQ4      4   // COM1
#define IRQ5      5   // LPT2 / 声卡
#define IRQ6      6   // 软盘控制器
#define IRQ7      7   // LPT1 / 并口
#define IRQ8      8   // RTC（实时时钟）
#define IRQ9      9   // ACPI / 通用
#define IRQ10     10  // 通用 / PCI
#define IRQ11     11  // 通用 / PCI
#define IRQ12     12  // PS/2鼠标
#define IRQ13     13  // FPU（浮点单元）
#define IRQ14     14  // 主IDE通道
#define IRQ15     15  // 从IDE通道

#include <Time.h>

void PIC_sendEOI(uint8_t irq);//发送中断结束信号(EOI)
void Init_PIC();//初始化PIC
void irq_enable(uint8_t irq_number);//启用IRQ
void irq_disable(uint8_t irq_number);//禁用IRQ
void set_irq();//设置IRQ中断门
void InitPIT(int f);//初始化PIT

__attribute__((naked)) void irq_handler_0();
__attribute__((naked)) void irq_handler_1();
__attribute__((naked)) void irq_handler_8();

extern volatile uint64_t timer_ticks;
void irq_0();
extern char IRQKey;
void irq_1();
extern uint32_t volatile conter;
void irq_8();

#endif