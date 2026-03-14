/*
    Task.h Task.c
 OS002 多线程管理
- 对内核线程的创建，上锁，睡眠，结束

2026/3/6 Liu Chunyi

*/

#ifndef TASK_H
#define TASK_H

#include <Tools.h>

typedef struct tss64 {
    uint32_t reserved0;          // 保留，必须为 0
    uint64_t rsp0;                // Ring 0 栈指针
    uint64_t rsp1;                // Ring 1 栈指针
    uint64_t rsp2;                // Ring 2 栈指针
    uint64_t reserved1;           // 保留，必须为 0
    uint64_t ist[7];              // 中断栈表（IST1～IST7），用于指定中断处理栈
    uint32_t reserved2;           // 保留，必须为 0
    uint32_t reserved3;           // 保留，必须为 0
    uint16_t reserved4;           // 保留，必须为 0
    uint16_t iomap_base;          // I/O 位图基址（通常设为 TSS 段界限，表示无位图）
} __attribute__((packed)) tss64_t;

typedef struct tss_task_t {
    tss64_t tss;
} tss_task_t;

// 线程状态
#define THREAD_READY      0
#define THREAD_RUNNING    1
#define THREAD_BLOCKED    2
#define THREAD_TERMINATED 3

#define FREE_TCB_LIST_SIZE 4
#define TORT_SIZE          16//正在运行的线程的表的大小

// 线程上下文（保存所有通用寄存器及 RIP、RSP、RFLAGS 等）
typedef struct thread_context {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    uint64_t rip;      // 指令指针（线程入口）
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;      // 栈指针
    uint64_t ss;
} __attribute__((packed)) thread_context_t;

// 线程控制块
typedef struct thread {
    thread_context_t context;
    uint64_t id;                // 线程 ID
    uint8_t state;              // 状态
    void *kernel_stack;         // 内核栈基址（用于分配）
    uint64_t stack_size;        // 栈大小
    struct thread *next;        // 就绪队列链表指针
    int PositionInTable;        // 线程在任务表中的位置
    uint64_t wakeup_time;       // 唤醒时间（用于睡眠）
}__attribute__((packed)) thread_t;

// 自旋锁类型
typedef struct spinlock {
    volatile uint8_t locked;  // 0: 未锁, 1: 已锁
} spinlock_t;

// 互斥量结构体
typedef struct mutex {
    volatile uint8_t locked;           // 0=未锁定，1=已锁定
    thread_t *owner;                   // 当前持有锁的线程
    thread_t *wait_queue_head;         // 等待队列头
    thread_t *wait_queue_tail;         // 等待队列尾
    volatile uint32_t lock_count;      // 锁计数器，用于递归锁
} mutex_t;

// 信号量结构体
typedef struct semaphore {
    volatile int count;              // 信号量计数
    thread_t *wait_queue_head;       // 等待队列头
    thread_t *wait_queue_tail;       // 等待队列尾
} semaphore_t;

// 初始化自旋锁
#define SPINLOCK_INIT {0}
static inline void spinlock_init(spinlock_t *lock) {
    lock->locked = 0;
}

// 全局当前运行线程
extern thread_t *current_thread;

// 函数声明
thread_t *create_kernel_thread(void (*entry)(void), uint64_t stack_size);//创建内核线程
void init_multitasking(void);//初始化多任务
void thread_enqueue(thread_t *thread);
thread_t *thread_dequeue(void);
void switch_to(thread_t *prev, thread_t *next);//切换线程
void schedule(void);
void spinlock_acquire(spinlock_t *lock);// 获取自旋锁
void spinlock_release(spinlock_t *lock);// 释放自旋锁
STATUS spinlock_try_acquire(spinlock_t *lock);// 尝试获取锁（非阻塞），成功返回1，失败返回0
void mutex_init(mutex_t *mutex);//初始化互斥量
void mutex_lock(mutex_t *mutex);//获取互斥量
void mutex_unlock(mutex_t *mutex);//释放互斥量
int mutex_is_owned_by_current(mutex_t *mutex); //检查当前线程是否拥有互斥量
void sem_init(semaphore_t *sem, int value);//初始化信号量
void sem_wait(semaphore_t *sem);//P操作（等待）
void sem_post(semaphore_t *sem);//V操作（发送信号）
void KillThread(thread_t* thread);//杀死线程
void PrintRunningThreads();//打印正在运行的线程
void ThreadSleepMS(uint64_t ms);//休眠指定毫秒数
void ThreadSleepSecond(uint64_t s);//休眠指定秒数
void wake_sleeping_threads(void); // 唤醒睡眠线程
void CleanAllThreads();//清理所有进程

#endif