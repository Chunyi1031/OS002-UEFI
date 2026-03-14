/*
中断描述符表库
*/

#ifndef INT_IDT_H
#define INT_IDT_H

#include <Tools.h>

#define INTERRUPT_GATE_SIZE     256

#define X86_64_INTERRUPT_GATE   0x8E //x86_64中断门
#define X86_64_TRAP_GATE        0x8F //x86_64陷阱门

//中断描述符表（IDT）结构体定义
typedef struct GATE_DESC {
    uint16_t offset_1;        //偏移0..15
    uint16_t selector;        //GDT和LDT选择子
    uint8_t  ist;             //位0至2为中断堆栈表偏移量，其余位为零。
    uint8_t  type_attributes; //门类型、DPL和P字段
    uint16_t offset_2;        //偏移16..31
    uint32_t offset_3;        //偏移32..63
    uint32_t reserved;        //保留，必须为零
} __attribute__((packed)) GATE_DESC;
//IDTR
typedef struct IDTR {
    uint16_t limit; //IDT表界限
    uint32_t base_l;  //IDT表基地址(低32位)
    uint32_t base_h;  //IDT表基地址(高32位)
} __attribute__((packed)) IDTR;

extern GATE_DESC idt_table[INTERRUPT_GATE_SIZE];//IDT表
extern uint16_t saved_cs, saved_ds;

STATUS Init_IDT();//初始化IDT
void init_pic();//初始化PIC
void set_intr_gate(uint32_t n, const void *addr,uint8_t type);//设置中断门
void set_irq_gate(uint32_t n, const void *addr);//设置IRQ
void lidt(uint64_t addr,uint16_t size);//加载IDT

void interrupt_handler_default(); //通用中断服务程序处理函数路口
void interrupt_handler_0(void);
void interrupt_handler_1(void);
void interrupt_handler_2(void);
void interrupt_handler_3(void);
void interrupt_handler_4(void);
void interrupt_handler_5(void);
void interrupt_handler_6(void);
void interrupt_handler_7(void);
void interrupt_handler_8(void);
void interrupt_handler_10(void);
void interrupt_handler_11(void);
void interrupt_handler_12(void);
void interrupt_handler_14(void);
void interrupt_handler_16(void);
void interrupt_handler_17(void);
void interrupt_handler_18(void);
void interrupt_handler_19(void);
void interrupt_handler_20(void);

void interrupt_default();//通用中断服务程序处理函数
void interrupt_DE();
void interrupt_DB();
void interrupt_NMI();
void interrupt_BP();
void interrupt_BR();
void interrupt_OF();
void interrupt_UD();
void interrupt_NM();
void interrupt_DF();
void interrupt_NP();
void interrupt_SS();
void interrupt_MF();
void interrupt_AC();
void interrupt_MC();
void interrupt_XM();
void interrupt_VE();
void interrupt_PF();

#endif