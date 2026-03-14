/*
参考:
    https://www.bilibili.com/video/BV1yMBqY2EcY?t=597.1
*/

#include <interrupt/PIC.h>
#include <Print.h>

volatile uint64_t timer_ticks = 0;//全局变量，用于记录时钟中断次数

//初始化PIT
void InitPIT(int f){
    outb(0x43, 0x34); //计数器0，模式2，先低后高，二进制
    uint16_t divisor = 1193180 / f; // 1193180 Hz / 100 Hz = 11931
    outb(0x40, divisor & 0xFF);       // 低字节
    outb(0x40, (divisor >> 8) & 0xFF); // 高字节
    timer_ticks = 0;//重置时钟中断次数
}

void Init_PIC(){
    InitPIT(PIT_CLOCK_FREQ);
    outb(PIC0_ICW1,PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC0_ICW2,IRQ_PIC_START);
    outb(PIC0_ICW3,1<<2);
    outb(PIC0_ICW4,PIC_ICW4_8086);
    outb(PIC1_ICW1,PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC1_ICW2,IRQ_PIC_START+8);
    outb(PIC1_ICW3,2);
    outb(PIC1_ICW4,PIC_ICW4_8086);
    outb(PIC0_IMR,0xFF & ~ (1 << 2));
    outb(PIC1_IMR, 0xFF);
    irq_enable(IRQ0);
}

//From https://wiki.osdev.org/8259_PIC
void PIC_sendEOI(uint8_t irq)
{
	if(irq >= 8)outb(PIC_2_CTRL,PIC_EOI);
	
	outb(PIC_1_CTRL,PIC_EOI);
}

void set_irq(){
    set_irq_gate(IRQ0, irq_handler_0);
    set_irq_gate(IRQ1, irq_handler_1);
    set_irq_gate(IRQ8, irq_handler_8);
}

void irq_enable(uint8_t irq_number){
    //主片
    if(irq_number < 8){
        uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_number);
        outb(PIC0_IMR, mask);
    //从片
    }else{
        irq_number -= 8;
        uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_number);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable(uint8_t irq_number){
    //主片
    if(irq_number < 8){
        uint8_t mask = inb(PIC0_IMR) | (1 << irq_number);
        outb(PIC0_IMR, mask);
    //从片
    }else{
        irq_number -= 8;
        uint8_t mask = inb(PIC1_IMR) | (1 << irq_number);
        outb(PIC1_IMR, mask);
    }
}

// 在 PIC.c 中添加
__attribute__((naked))
void irq_handler_0(void) {
    __asm__ volatile(
        "cli\n"
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

        "sub $8, %rsp\n"          // 调整栈对齐（15个寄存器120字节，+8=128，16的倍数）
        "call irq_0\n"
        "add $8, %rsp\n"          // 恢复

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

        "iretq"
    );
}

#include <Task.h>
void irq_0(void) {
    if(timer_ticks >= 0xFFFFFFFFFFFFFFFF){
        timer_ticks = 0;
    }else{
        timer_ticks++;
    }
    PIC_sendEOI(IRQ0);//发送 EOI
    // 只在主调度循环中调用唤醒睡眠线程的函数，避免在中断上下文中执行
    schedule();//调用调度器，可能切换线程
}

__attribute__((naked))
void irq_handler_1(void) {
    __asm__ volatile(
        "cli\n"
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

        "sub $8, %rsp\n"          // 调整栈对齐（120+8=128，16的倍数）
        "call irq_1\n"
        "add $8, %rsp\n"          // 恢复

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

        "iretq"
    );
}


#include <Keyboard.h>
char IRQKey = 0;
void irq_1(){
    IRQKey = inb(0x60);
    PIC_sendEOI(IRQ1);
}


__attribute__((naked))
void irq_handler_8(void) {
    __asm__ volatile(
        "cli\n"
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

        "sub $8, %rsp\n"          // 调整栈对齐（120+8=128，16的倍数）
        "call irq_8\n"
        "add $8, %rsp\n"          // 恢复

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

        "iretq"
    );
}

uint32_t volatile conter = 0;
void irq_8(){
    PIC_sendEOI(IRQ8);
    printf("%d\n",conter++);
}