/*
    Tools.h Tools.c
 OS002基础库
- 定义系统基本函数，以及对系统的操作，延迟，内联汇编，数学运算

2026/3/6 Liu Chunyi

*/

#ifndef TOOLS_H
#define TOOLS_H

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;
typedef uint64_t           size_t;
typedef uint64_t           uintptr_t;

typedef uint8_t            STATUS;

#define STATUS_SUCCESS 0x00
#define NULL ((void*)0)

//映射信息
typedef struct SCREEN_DATA{
    uint64_t FrameBuffer;
    uint16_t SW;
    uint16_t SH;
}SCREEN_DATA;
typedef struct MEMORY_MAP{
    uint64_t MapSize;
    uint64_t DescriptorSize;
    uint32_t DescriptorVersion;
    void* StackAddr;
    void* Buffer;
}MEMORY_MAP;
typedef struct BOOT_SHARE{
    SCREEN_DATA* ScreenData;
    uint64_t CPUF;
    uint32_t FONTADDR;
    void *RSDP;
    MEMORY_MAP* MemoryMap;
} BOOT_SHARE;

extern uint64_t CPU_Frequency;

//IO端口
void outb(uint16_t port, uint8_t value);//向端口输出1字节
void outw(uint16_t port, uint16_t val);//向端口输出2字节
void outl(uint16_t port, uint32_t val);//向端口输出4字节
uint8_t inb(uint16_t port);//从端口读1字节
uint16_t inw(uint16_t port);//从端口读2字节
uint32_t inl(uint16_t port);//从端口读4字节
uint64_t rdtsc(void);//读取开机以来CPU振荡的次数
uint64_t get_rip(void);//获取RIP指针
void io_wait(void);//IO等待

void SYSTEM_STOP();//停机
uint64_t Get_CPU_Frequency();//获取CPU频率

void delay_ms(uint32_t ms);//延迟毫秒
void delay_seconds(uint32_t seconds);//延迟秒

size_t strlen(const char *s);//计算字符串长度
size_t itoa_dec(int64_t value, char *buf);//将有符号整数转为十进制字符串
size_t itoa_hex(uint64_t value, char *buf);//将无符号整数转为十六进制字符串
char LastCharOf(char* s);//...的最后一个字符
size_t strcount(const char *str, char ch);//字符在字符串中出现的次数
void reverse(char *str, int length);//反转字符串
int strcmp(char* str1,char* str2);//比较字符串
int memcmp(const void* s1, const void* s2, size_t n);//逐字节比较两块内存
/**
 * strnlen - Find the length of a length-limited string
 * @s: The string to be sized
 * @count: The maximum number of bytes to search
 */
size_t strnlen(const char *s, size_t count);
/**
 * memcpy - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
void *memcpy(void *dest, const void *src, size_t count);

//中断控制函数声明
void enable_interrupts(void);
void disable_interrupts(void);
uint8_t are_interrupts_enabled(void);

//串口函数声明
#define SERIAL_BAUD 115200
#define SERIAL_COM1_PORT 0x3F8
#define SERIAL_COM2_PORT 0x2F8
#define SERIAL_COM3_PORT 0x3E8
STATUS serial_init(uint16_t port);//初始化串口
int is_transmit_empty(uint16_t port);//检查串口是否准备好发送数据
void serial_putchar(uint16_t port, char c);//发送单个字符到串口
void serial_putstr(uint16_t port, const char* str);//发送字符串到串口


/**
 * memset - Fill a region of memory with the given value
 * @s: Pointer to the start of the area.
 * @c: The byte to fill the area with
 * @count: The size of the area.
 *
 * Do not use memset() to access IO space, use memset_io() instead.
 */
void *memset(void *s, int c, size_t count);
/**
 * memset16() - Fill a memory area with a uint16_t
 * @s: Pointer to the start of the area.
 * @v: The value to fill the area with
 * @count: The number of values to store
 *
 * Differs from memset() in that it fills with a uint16_t instead
 * of a byte.  Remember that @count is the number of uint16_ts to
 * store, not the number of bytes.
 */
void *memset16(uint16_t *s, uint16_t v, size_t count);

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void init_gdt();//初始化GDT

void beep(uint32_t frequency, uint32_t duration_ms);// 控制蜂鸣器发声

#endif