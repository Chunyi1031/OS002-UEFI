/*
参考:
    https://mirrors.kernel.org/pub/linux/kernel/v5.x/linux-5.2.20.tar.xz
    https://github.com/Chunyi1031/OS001-UEFI/blob/main/Kernel/kernel/Tools.c
*/

#include <Tools.h>

void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory");
}
uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
//写16位到I/O端口
void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}
// 往端口写入一个双字
void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %1, %0" : : "Nd"(port), "a"(val));
}

// IO等待延迟
void io_wait(void) {
    outb(0x80, 0);  // 往诊断端口写入任意值以产生IO延迟
}
uint64_t get_rip(void) {
    uint64_t rip;
    //call 指令会将返回地址（下一条指令）压栈
    asm volatile (
        "leaq (%%rip), %0\n\t" 
        "call 1f\n\t"           // 1: 是局部标签，call 会跳转到它
        "1:\n\t"                // 这是 call 要跳转到的位置
        "pop %0"                // 将返回地址（即此处的地址）弹出到变量
        : "=r" (rip)
        :
        : "memory"
    );
    return rip;
}
uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ (
        "rdtsc"
        : "=a"(lo), "=d"(hi)
    );
    return ((uint64_t)hi << 32) | lo;
}

void SYSTEM_STOP(){
    //无限循环
    while(1){
        asm volatile("hlt");//减小CPU占用
    }
}

#include <Time.h>
//获取CPU频率
uint64_t Get_CPU_Frequency(){
    struct rtc_time time;
    //等待下一个整秒
    get_rtc_time(&time);
    uint16_t s1 = time.second;
    uint16_t s2 = 0;
    while (s2 != (s1 + 1)){
        get_rtc_time(&time);
        s2 = time.second;
        asm("pause");
    }
    uint64_t tsc1 = rdtsc();//第一次获取CPU振荡次数
    //等待下一秒
    uint16_t s3 = 0;
    while (s3 != (s2 + 1)){
        get_rtc_time(&time);
        s3 = time.second;
        asm("pause");
    }
    uint64_t tsc2 = rdtsc();//第二次
    return tsc2 - tsc1;//两次相距1秒，两次的相减即为频率
}

//延迟毫秒
void delay_ms(uint32_t ms) {
    uint64_t cycles_to_wait = (uint64_t)ms * (CPU_Frequency / 1000);//计算需要等待的总周期数
    uint64_t start_tsc = rdtsc();//记录起始点
    //循环检查经过的周期数是否达到目标
    while ((rdtsc() - start_tsc) < cycles_to_wait) {
        asm volatile("pause");//pause降低功耗
    }
}
//延迟秒
void delay_seconds(uint32_t seconds){
    delay_ms(seconds * 1000);
}


//将无符号整数转为十六进制字符串
size_t itoa_hex(uint64_t value, char *buf) {
    char temp[32];
    int i = 0;
    // 特殊处理 0
    if (value == 0) {
        buf[0] = '0';
        buf[1] = 'x';
        buf[2] = '0';
        buf[3] = '\0';
        return 3;
    }
    // 逆序生成 hex
    while (value > 0) {
        int digit = value & 0xF;
        temp[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        value >>= 4;
    }
    // 写 "0x"
    buf[0] = '0';
    buf[1] = 'x';
    int j = 2;
    // 反向拷贝
    while (i > 0) {
        buf[j++] = temp[--i];
    }
    buf[j] = '\0';
    return (size_t)j;
}
//将有符号整数转为十进制字符串
size_t itoa_dec(int64_t value, char *buf) {
    char temp[32];
    int i = 0;
    uint64_t num;
    int negative = 0;
    if (value < 0) {
        negative = 1;
        num = (uint64_t)(-value);
    } else {
        num = (uint64_t)value;
    }
    // 特殊处理 0
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }
    // 逆序生成数字
    while (num > 0) {
        temp[i++] = '0' + (num % 10);
        num /= 10;
    }
    int j = 0;
    // 先写负号（如果有）
    if (negative) {
        buf[j++] = '-';
    }
    // 再反向拷贝数字
    while (i > 0) {
        buf[j++] = temp[--i];
    }
    buf[j] = '\0';

    return (size_t)j;
}

//...的最后一个字符
char LastCharOf(char* s){
    size_t len = strlen(s) - 1;
    return s[len];
}
//字符在字符串中出现的次数
size_t strcount(const char *str, char ch) {
    size_t count = 0;
    if (str == NULL) {
        return 0;
    }
    //遍历字符串
    while (*str != '\0') {
        if (*str == ch) {
            count++;
        }
        str++;
    }
    return count;
}
//反转字符串
void reverse(char *str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

//From Linux 5.2.20  lib/string.c 512
size_t strlen(const char *s) {
    const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}
//From Linux 5.2.20  lib/string.c 529
size_t strnlen(const char *s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return (int)p1[i] - (int)p2[i];  // 返回差值
        }
    }
    return 0;  // 完全相同
}

int strcmp(char* str1,char* str2){
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    if(len1 == 0 && len2 == 0) return 0;
    if((len1 == 0)||(len2 == 0)) return 1;
    if(len1 >= len2){
        for(int i = 0;i < len2;i ++){
            if(str1[i] != str2[i]){
                return 2;
            }
        }
        if(len1 != len2) return 1;
        return 0;
    }else{
        for(int i = 0;i < len1;i ++){
            if(str1[i] != str2[i]){
                return 2;
            }
        }
        return 1;
    }
}

// 启用中断
void enable_interrupts(void) {
    asm volatile ("sti");
}

// 禁用中断
void disable_interrupts(void) {
    asm volatile ("cli");
}

// 检查中断是否启用
uint8_t are_interrupts_enabled(void) {
    uint64_t rflags;
    asm volatile ("pushf\n\tpop %0" : "=rm"(rflags));
    return (rflags & (1 << 9)) != 0;  // IF标志位在第9位
}


//串口初始化
STATUS serial_init(uint16_t port){
    outb(port + 1, 0x00);//禁用中断
    outb(port + 3, 0x80);//启用 DLAB
    outb(port + 0, (1843200 / (16*SERIAL_BAUD)));//设置波特率
    outb(port + 1, 0x00);
    outb(port + 3, 0x03);//8位数据
    outb(port + 2, 0xC7);//启用 FIFO
    outb(port + 4, 0x0B);
    outb(port + 4, 0x1E);
    outb(port + 0, 0xAE);
    if (inb(port + 0) != 0xAE) {
        return 1;
    }
    outb(port + 4, 0x0F);
    return STATUS_SUCCESS;
}
// 检查串口是否准备好发送数据
int is_transmit_empty(uint16_t port) {
    return inb(port + 5) & 0x20;
}
// 发送单个字符到串口
void serial_putchar(uint16_t port, char c) {
    while (!is_transmit_empty(port));  // 等待发送缓冲区为空
    outb(port, c);  // 发送字符
}
// 发送字符串到串口
void serial_putstr(uint16_t port, const char* str) {
    while (*str) {
        serial_putchar(port, *str++);
    }
}

//From Linux 5.2.20  lib/string.c 729
void *memset(void *s, int c, size_t count)
{
	char *xs = (char *)s;

	while (count--)
		*xs++ = c;
	return s;
}
//From Linux 5.2.20  lib/string.c 772
void *memset16(uint16_t *s, uint16_t v, size_t count)
{
	uint16_t *xs = s;

	while (count--)
		*xs++ = v;
	return s;
}
//From Linux 5.2.20  lib/string.c 837
void *memcpy(void *dest, const void *src, size_t count)
{
	char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}

struct gdt_entry gdt_table[3];
struct gdt_ptr gdtr;

void init_gdt() {
    gdt_table[0] = (struct gdt_entry){0}; // 空描述符
    gdt_table[1] = (struct gdt_entry){
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_middle = 0,
        .access = 0x9A, // 代码段
        .granularity = 0xAF,
        .base_high = 0
    };
    gdt_table[2] = (struct gdt_entry){
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_middle = 0,
        .access = 0x92, // 数据段
        .granularity = 0xAF,
        .base_high = 0
    };

    gdtr.limit = sizeof(gdt_table) - 1;
    gdtr.base = (uint64_t)&gdt_table;

    asm volatile("lgdt %0" :: "m"(gdtr));
}

// 控制蜂鸣器发声
void beep(uint32_t frequency, uint32_t duration_ms) {
    // 计算计数器值
    uint32_t counter = 1193180 / frequency;

    // 设置 PIT 通道2为模式3（方波发生器）
    outb(0x43, 0xB6);              // 0xB6 = 10110110: 通道2, 模式3, 先低后高, 二进制
    outb(0x42, counter & 0xFF);     // 写入低字节
    outb(0x42, (counter >> 8) & 0xFF); // 写入高字节

    // 启用蜂鸣器
    uint8_t status = inb(0x61);
    outb(0x61, status | 0x03);      // 设置位0和位1启用蜂鸣器

    // 延迟指定时间
    // 这里假设有一个毫秒级延时函数 delay_ms()
    delay_ms(duration_ms);

    // 关闭蜂鸣器
    outb(0x61, status & 0xFC);      // 清除位0和位1关闭蜂鸣器
}