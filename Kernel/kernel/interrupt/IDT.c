/*
参考:
    https://www.bilibili.com/video/BV1VZScYoEH4?t=882.5
    https://www.bilibili.com/video/BV1GZScYoESq?t=2.2
*/

#include <Interrupt.h>
#include <interrupt/IDT.h>
#include <interrupt/PIC.h>
#include <Print.h>
#include <Draw.h>

GATE_DESC idt_table[INTERRUPT_GATE_SIZE];

uint16_t saved_cs, saved_ds;

void set_intr_gate(uint32_t n, const void *addr,uint8_t type) {
    GATE_DESC* entry = &idt_table[n];
    entry->offset_1 = (uint16_t)((uint64_t)addr & 0xFFFF);           // 低16位
    entry->selector = saved_cs;
    entry->ist = 0;
    entry->type_attributes = type;
    entry->offset_2 = (uint16_t)(((uint64_t)addr >> 16) & 0xFFFF);   // 中间16位
    entry->offset_3 = (uint32_t)(((uint64_t)addr >> 32) & 0xFFFFFFFF); // 高32位
    entry->reserved = 0;
}

void set_irq_gate(uint32_t n, const void *addr){
    set_intr_gate(IRQ_PIC_START + n, addr,X86_64_INTERRUPT_GATE);
}

//加载IDT
void lidt(uint64_t addr,uint16_t size){
    IDTR idtr; //IDTR寄存器
    idtr.limit = size - 1; //IDT表界限
    idtr.base_h = (uint32_t)((addr >> 32) & 0xFFFFFFFF); //IDT表基地址(高32位)
    idtr.base_l = (uint32_t)(addr & 0xFFFFFFFF); //IDT表基地址(低32位)
    __asm__ volatile("lidt %0" : : "m"(idtr)); //加载IDT
}

//初始化IDT
STATUS Init_IDT(){
    __asm__ volatile("mov %%cs, %0" : "=r"(saved_cs));
    __asm__ volatile("mov %%ds, %0" : "=r"(saved_ds));
    //循环设置IDT表中每个中断向量的默认处理函数
    for(int i = 0;i < INTERRUPT_GATE_SIZE;i ++){
        set_intr_gate(i, interrupt_handler_default,X86_64_INTERRUPT_GATE);
    }
    //特殊设置
    set_intr_gate(IDT_DE_VECTOR,interrupt_handler_0,X86_64_INTERRUPT_GATE);//除零异常
    set_intr_gate(IDT_DB_VECTOR,interrupt_handler_1,X86_64_INTERRUPT_GATE);//调试异常
    set_intr_gate(IDT_NMI_VECTOR,interrupt_handler_2,X86_64_INTERRUPT_GATE);//非屏蔽中断
    set_intr_gate(IDT_BP_VECTOR,interrupt_handler_3,X86_64_INTERRUPT_GATE);//断点异常
    set_intr_gate(IDT_OF_VECTOR,interrupt_handler_4,X86_64_INTERRUPT_GATE);//内存溢出
    set_intr_gate(IDT_BR_VECTOR,interrupt_handler_5,X86_64_INTERRUPT_GATE);//栈溢出
    set_intr_gate(IDT_UD_VECTOR,interrupt_handler_6,X86_64_INTERRUPT_GATE);//无效操作码
    set_intr_gate(IDT_NM_VECTOR,interrupt_handler_7,X86_64_INTERRUPT_GATE);//设备不可用
    set_intr_gate(IDT_DF_VECTOR,interrupt_handler_8,X86_64_INTERRUPT_GATE);//双重故障
    set_intr_gate(IDT_TS_VECTOR,interrupt_handler_10,X86_64_INTERRUPT_GATE);//无效TSS
    set_intr_gate(IDT_SS_VECTOR,interrupt_handler_12,X86_64_INTERRUPT_GATE);//堆栈段错误
    set_intr_gate(IDT_PF_VECTOR,interrupt_handler_14,X86_64_INTERRUPT_GATE);//页错误
    set_intr_gate(IDT_MF_VECTOR,interrupt_handler_16,X86_64_INTERRUPT_GATE);//x87 浮点异常
    set_intr_gate(IDT_AC_VECTOR,interrupt_handler_17,X86_64_INTERRUPT_GATE);//浮点异常
    set_intr_gate(IDT_MC_VECTOR,interrupt_handler_18,X86_64_INTERRUPT_GATE);//机器检查
    set_intr_gate(IDT_XM_VECTOR,interrupt_handler_19,X86_64_INTERRUPT_GATE);//SIMD浮点异常
    set_intr_gate(IDT_VE_VECTOR,interrupt_handler_20,X86_64_INTERRUPT_GATE);//虚拟化异常
    set_irq();//设置IRQ
    lidt((uint64_t)&idt_table, INTERRUPT_GATE_SIZE * 16); //加载IDT
    Init_PIC();
    io_wait();
    enable_interrupts();//开启中断
}



/*中断处理**********************************************************/


//默认中断处理函数入口
__attribute__((naked))
void interrupt_handler_default(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_default\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}

//除法错误中断处理函数入口
__attribute__((naked))
void interrupt_handler_0(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_DE\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}

__attribute__((naked))
void interrupt_handler_1(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_DB\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}

__attribute__((naked))
void interrupt_handler_2(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_BP\n"
    );
}

//断点异常中断处理函数入口
__attribute__((naked))
void interrupt_handler_3(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_BP\n"
    );
}

__attribute__((naked))
void interrupt_handler_4(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_OF\n"
    );
}

__attribute__((naked))
void interrupt_handler_5(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_BR\n"
    );
}

__attribute__((naked))
void interrupt_handler_6(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_UD\n"
    );
}

__attribute__((naked))
void interrupt_handler_7(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_NM\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}

__attribute__((naked))
void interrupt_handler_8(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_DF\n"
    );
}

__attribute__((naked))
void interrupt_handler_10(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_TS\n"
    );
}

//除法错误中断处理函数入口
__attribute__((naked))
void interrupt_handler_11(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_NP\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}

__attribute__((naked))
void interrupt_handler_12(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_SS\n"
    );
}

__attribute__((naked))
void interrupt_handler_16(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_MF\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}

__attribute__((naked))
void interrupt_handler_14(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_PF\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}

__attribute__((naked))
void interrupt_handler_17(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_AC\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}


__attribute__((naked))
void interrupt_handler_18(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "jmp interrupt_MC\n"
    );
}

__attribute__((naked))
void interrupt_handler_19(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_XM\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}


__attribute__((naked))
void interrupt_handler_20(void) {
    __asm__ volatile (
        "cli\n"
        // 保存所有通用寄存器（15个）
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rbx\n"
        "pushq %rbp\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "sub $8, %rsp\n" 

        "call interrupt_VE\n"

        // 丢弃 call 压入的返回地址（因为我们要用 iretq 返回）
        "addq $8, %rsp\n"

        // 恢复所有通用寄存器（顺序与 push 相反）
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rbp\n"
        "popq %rbx\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        "sti\n"

        // 从中断返回
        "iretq"
    );
}


//默认处理函数
void interrupt_default(){
}

//除0错误处理函数
void interrupt_DE(){
    out_error("#DE");
    serial_putstr(SERIAL_COM1_PORT, "#DE");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#DE",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_DB(){
    out_error("#DB");
    serial_putstr(SERIAL_COM1_PORT, "#DB");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#DB",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_NMI(){
    out_error("#NMI");
    serial_putstr(SERIAL_COM1_PORT, "#NMI");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#NMI",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

//断点错误处理函数
void interrupt_BP(){
    out_error("#BP");
    serial_putstr(SERIAL_COM1_PORT, "#BP");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#BP",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_OF(){
    out_error("#OF");
    serial_putstr(SERIAL_COM1_PORT, "#OF");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#OF",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_PF(){
    out_error("#PF");
    serial_putstr(SERIAL_COM1_PORT, "#PF");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#PF",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_BR(){
    out_error("#BR");
    serial_putstr(SERIAL_COM1_PORT, "#BR");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#BR",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_UD(){
    out_error("#UD");
    serial_putstr(SERIAL_COM1_PORT, "#UD");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#UD",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_NM(){
    out_error("#NM");
    serial_putstr(SERIAL_COM1_PORT, "#NM");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#NM",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_DF(){
    out_error("#DF");
    serial_putstr(SERIAL_COM1_PORT, "#DF");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#DF",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_TS(){
    out_error("#TS");
    serial_putstr(SERIAL_COM1_PORT, "#TS");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#TS",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_NP(){
    out_error("#NP");
    serial_putstr(SERIAL_COM1_PORT, "#NP");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#NP",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_SS(){
    out_error("#SS");
    serial_putstr(SERIAL_COM1_PORT, "#SS");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#SS",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_MF(){
    out_error("#MF");
    serial_putstr(SERIAL_COM1_PORT, "#MF");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#MF",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_AC(){
    out_error("#AC");
    serial_putstr(SERIAL_COM1_PORT, "#AC");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#AC",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_MC(){
    out_error("#MC");
    serial_putstr(SERIAL_COM1_PORT, "#MC");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#MC",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_XM(){
    out_error("#XM");
    serial_putstr(SERIAL_COM1_PORT, "#XM");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#XM",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}

void interrupt_VE(){
    out_error("#VE");
    serial_putstr(SERIAL_COM1_PORT, "#VE");
    delay_ms(100);
    fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_RED);
    DrawString("Your computer is experiencing a problem and needs to be restarted:#VE",(ScreenWidth / 2) - (70 * 5),(ScreenHeigth / 2) - 8,COLOR_WHITE);
    delay_ms(100);
    SYSTEM_STOP();
}
