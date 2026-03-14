#include <Task.h>
#include <Memory.h>
#include <Print.h>
#include <interrupt/PIC.h>

thread_t *current_thread = NULL;
static uint64_t next_thread_id = 1;
static int next_TORT_Position = 0;
static uintptr_t free_tcb_list[FREE_TCB_LIST_SIZE]; //空闲TCB链表
static uint32_t num_of_FreeTCB = 0;
static uintptr_t TORT[TORT_SIZE];//正在运行的线程的表

// 睡眠队列
static thread_t *sleep_queue_head = NULL;
static thread_t *sleep_queue_tail = NULL;
// 就绪队列头指针
static thread_t *ready_queue_head = NULL;
static thread_t *ready_queue_tail = NULL;  // 方便尾部插入

void idle_thread(void) {
    while(1) {
        asm volatile("hlt");
    }
}

// 初始化多任务环境（创建 idle 线程或主线程）
void init_multitasking(void) {
    memset(TORT, 0, sizeof(TORT));//清空TORT
    memset(free_tcb_list, 0, sizeof(free_tcb_list));//清空空闲TCB链表
    // 初始化队列头指针
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    sleep_queue_head = NULL;
    sleep_queue_tail = NULL;
    current_thread = (thread_t *)pmm_alloc_pages(1);
    memset(current_thread, 0, sizeof(thread_t));
    current_thread->id = 0;
    current_thread->state = THREAD_RUNNING;
    create_kernel_thread(idle_thread, 4096);
}

void thread_enqueue(thread_t *thread) {
    thread->next = NULL;
    if (!ready_queue_head) {
        ready_queue_head = ready_queue_tail = thread;
    } else {
        ready_queue_tail->next = thread;
        ready_queue_tail = thread;
    }
    thread->state = THREAD_READY;
}

// 睡眠队列专用的入队函数
void sleep_enqueue(thread_t *thread) {
    thread->next = NULL;
    if (!sleep_queue_head) {
        sleep_queue_head = sleep_queue_tail = thread;
    } else {
        sleep_queue_tail->next = thread;
        sleep_queue_tail = thread;
    }
}

// 睡眠队列专用的出队函数
thread_t *sleep_dequeue(void) {
    if (!sleep_queue_head) return NULL;
    thread_t *thread = sleep_queue_head;
    sleep_queue_head = sleep_queue_head->next;
    if (!sleep_queue_head) sleep_queue_tail = NULL;
    thread->next = NULL;
    return thread;
}

// 从睡眠队列中移除指定线程
thread_t *remove_from_sleep_queue(thread_t *target) {
    if (!sleep_queue_head) return NULL;
    if (sleep_queue_head == target) {
        return sleep_dequeue();
    }
    
    thread_t *curr = sleep_queue_head;
    while (curr && curr->next != target) {
        curr = curr->next;
    }
    
    if (curr && curr->next == target) {
        thread_t *removed = target;
        curr->next = target->next;
        if (target == sleep_queue_tail) {
            sleep_queue_tail = curr;
        }
        removed->next = NULL;
        return removed;
    }
    return NULL;
}

thread_t *thread_dequeue(void) {
    if (!ready_queue_head) return NULL;
    thread_t *thread = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    if (!ready_queue_head) ready_queue_tail = NULL;
    thread->next = NULL;
    return thread;
}

void thread_exit(void) {
    disable_interrupts();
    current_thread->state = THREAD_TERMINATED;
    // 从睡眠队列中移除（如果在睡眠队列中）
    remove_from_sleep_queue(current_thread);
    
    // 释放内核栈
    if (current_thread->kernel_stack) {
        pmm_free_pages(current_thread->kernel_stack,
                       current_thread->stack_size / PAGE_SIZE);
        current_thread->kernel_stack = NULL;
    }
    //如果未满，则继续添加
    if(num_of_FreeTCB < FREE_TCB_LIST_SIZE){
        num_of_FreeTCB ++;
        free_tcb_list[num_of_FreeTCB] = (uintptr_t)current_thread;
    }
    TORT[current_thread->PositionInTable] = 0;
    schedule();//调度其他线程
    SYSTEM_STOP();
}

// 创建一个内核线程
thread_t *create_kernel_thread(void (*entry)(void), uint64_t stack_size) {
    //分配内核栈
    stack_size = (stack_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    thread_t *thread = NULL;
    //如果链表有空闲的 TCB，则从链表中获取
    if (free_tcb_list[num_of_FreeTCB] != 0) {
        thread = (thread_t*)free_tcb_list[num_of_FreeTCB];//设置TCB
        memset(thread, 0, sizeof(thread_t));//清空
        free_tcb_list[num_of_FreeTCB] = 0;//已被占用，把该地址清除
        num_of_FreeTCB --;//回退
    //否则
    } else {
        thread = (thread_t *)pmm_alloc_pages(1);//再开辟一个TCB
    }
    if (!thread)return NULL;
    void *stack = pmm_alloc_pages(stack_size / PAGE_SIZE);
    if (!stack) {
        pmm_free_pages(thread, 1);
        return NULL;
    }
    memset(thread, 0, sizeof(thread_t));
    // 初始化 TCB
    thread->id = next_thread_id++;
    thread->state = THREAD_READY;
    thread->kernel_stack = stack;
    thread->stack_size = stack_size;
    if(next_TORT_Position < TORT_SIZE){
        thread->PositionInTable = next_TORT_Position;
        TORT[next_TORT_Position] = (uintptr_t)thread;//加入表
        next_TORT_Position ++;//下一个位置
    } else {
        out_error("Thread too many\n");
        return NULL;
    }
    uint64_t stack_bottom = (uint64_t)stack + stack_size;// 计算栈底（高地址）
    // 预留 17 个槽位（16 个寄存器 + rip），并 16 字节对齐
    uint64_t stack_frame = (stack_bottom - 17 * 8) & ~15ULL;
    uint64_t *frame = (uint64_t *)stack_frame;
    // 初始化栈帧：所有通用寄存器初始为 0，rflags = 0x202，rip = entry
    memset(frame, 0, 17 * 8);
    frame[15] = 0x202;          // rflags
    frame[16] = (uint64_t)entry; // rip
    frame[17] = (uint64_t)thread_exit; // 返回地址
    thread->context.rsp = stack_frame;// 设置线程上下文的 rsp
    thread_enqueue(thread);
    return thread;
}

__attribute__((naked))
void switch_to(thread_t *prev, thread_t *next) {
    __asm__ volatile (
        // 保存当前寄存器到 prev 的栈
        "pushfq\n"
        "pushq %%rax\n"
        "pushq %%rcx\n"
        "pushq %%rdx\n"
        "pushq %%rbx\n"
        "pushq %%rbp\n"
        "pushq %%rsi\n"
        "pushq %%rdi\n"
        "pushq %%r8\n"
        "pushq %%r9\n"
        "pushq %%r10\n"
        "pushq %%r11\n"
        "pushq %%r12\n"
        "pushq %%r13\n"
        "pushq %%r14\n"
        "pushq %%r15\n"

        // 保存当前栈指针到 prev->context.rsp（偏移 0x90）
        "movq %%rsp, 0x90(%%rdi)\n"

        // 切换到 next 的栈
        "movq 0x90(%%rsi), %%rsp\n"

        // 从新栈恢复所有寄存器
        "popq %%r15\n"
        "popq %%r14\n"
        "popq %%r13\n"
        "popq %%r12\n"
        "popq %%r11\n"
        "popq %%r10\n"
        "popq %%r9\n"
        "popq %%r8\n"
        "popq %%rdi\n"
        "popq %%rsi\n"
        "popq %%rbp\n"
        "popq %%rbx\n"
        "popq %%rdx\n"
        "popq %%rcx\n"
        "popq %%rax\n"
        "popfq\n"

        "ret\n"
        :
        : "D"(prev), "S"(next)
        : "memory"
    );
}

void schedule(void) {
    // 如果当前线程正在运行，放回就绪队列
    if (current_thread && current_thread->state == THREAD_RUNNING) {
        current_thread->state = THREAD_READY;
        thread_enqueue(current_thread);
    }
    // 唤醒到达时间的睡眠线程 - 移至此处避免中断上下文中的竞态条件
    wake_sleeping_threads();
    thread_t *next = thread_dequeue();
    if (!next) {
        //没有就绪线程，继续运行当前（可能是 idle）
        return;
    }
    thread_t *prev = current_thread;
    current_thread = next;
    current_thread->state = THREAD_RUNNING;
    if (prev != next) {
        switch_to(prev, next);
    }
}

// 获取自旋锁
void spinlock_acquire(spinlock_t *lock) {
    // 关中断以防止死锁（如果当前线程持有锁时被中断，中断处理程序也可能尝试获取同一把锁）
    disable_interrupts();

    // 使用 xchg 指令原子性地交换
    asm volatile (
        "1: xchgb %0, %1\n"
        "   testb %0, %0\n"
        "   jz 2f\n"
        "   pause\n"
        "   jmp 1b\n"
        "2:\n"
        : "+r" (*(uint8_t*)&lock->locked), "+m" (lock->locked)
        :
        : "memory"
    );
}

// 释放自旋锁
void spinlock_release(spinlock_t *lock) {
    // 使用 xchg 清0
    asm volatile (
        "xchgb %0, %1\n"
        : "+r" (*(uint8_t*)&lock->locked), "+m" (lock->locked)
        :
        : "memory"
    );
    
    // 重新开启中断
    enable_interrupts();
}

// 尝试获取锁
STATUS spinlock_try_acquire(spinlock_t *lock) {
    uint8_t old = 1;
    asm volatile (
        "xchgb %0, %1\n"
        : "+r" (old), "+m" (lock->locked)
        :
        : "memory"
    );
    return old == 0;  // 如果原来为0，则获取成功
}

static void thread_enqueue_wait(mutex_t *mutex, thread_t *thread) {
    thread->next = NULL;
    if (!mutex->wait_queue_head) {
        mutex->wait_queue_head = mutex->wait_queue_tail = thread;
    } else {
        mutex->wait_queue_tail->next = thread;
        mutex->wait_queue_tail = thread;
    }
}

static thread_t *thread_dequeue_wait(mutex_t *mutex) {
    if (!mutex->wait_queue_head) return NULL;
    thread_t *thread = mutex->wait_queue_head;
    mutex->wait_queue_head = thread->next;
    if (!mutex->wait_queue_head) mutex->wait_queue_tail = NULL;
    thread->next = NULL;
    return thread;
}

void mutex_init(mutex_t *mutex) {
    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->wait_queue_head = mutex->wait_queue_tail = NULL;
}

void mutex_lock(mutex_t *mutex) {
    disable_interrupts();   // 关中断，保证原子性
    
    // 检查是否为递归锁（同一线程再次获取锁）
    if (mutex->owner == current_thread) {
        // 如果是递归锁，可以增加计数器（这里简化处理，只做基本检查）
        enable_interrupts();
        return;
    }
    
    while (mutex->locked) {
        // 锁已被占用，当前线程进入等待队列并让出CPU
        current_thread->state = THREAD_BLOCKED;
        thread_enqueue_wait(mutex, current_thread);
        schedule();          // 切换出去，返回时中断仍为关
        // 被唤醒后，重新检查锁状态（可能又被其他线程抢走，故用while循环）
    }
    // 成功获得锁
    mutex->locked = 1;
    mutex->owner = current_thread;
    enable_interrupts();
}

void mutex_unlock(mutex_t *mutex) {
    disable_interrupts();
    
    // 检查是否当前线程拥有锁
    if (mutex->owner != current_thread) {
        // 如果不是当前线程持有锁，直接返回（可选：输出错误信息）
        enable_interrupts();
        return;
    }
    
    mutex->locked = 0;
    mutex->owner = NULL;
    // 唤醒一个等待线程（如果有）
    thread_t *waiter = thread_dequeue_wait(mutex);
    if (waiter) {
        waiter->state = THREAD_READY;
        thread_enqueue(waiter);   // 放回就绪队列
    }
    enable_interrupts();
}

// 添加递归锁功能：检查当前线程是否拥有锁
int mutex_is_owned_by_current(mutex_t *mutex) {
    return mutex->owner == current_thread;
}

void KillThread(thread_t* thread) {
    if (!thread) return; //参数校验
    disable_interrupts(); //关中断，防止并发问题
    //标记线程为终止状态
    thread->state = THREAD_TERMINATED;
    
    // 从睡眠队列中移除（如果在线程中）
    remove_from_sleep_queue(thread);
    
    //如果是当前运行线程，则立即调度
    if (thread == current_thread) {
        //释放内核栈
        if (thread->kernel_stack) {
            pmm_free_pages(thread->kernel_stack, thread->stack_size / PAGE_SIZE);
            thread->kernel_stack = NULL;
        }
        //将 TCB 加入空闲链表
        if (num_of_FreeTCB < FREE_TCB_LIST_SIZE) {
            num_of_FreeTCB++;
            free_tcb_list[num_of_FreeTCB] = (uintptr_t)thread;
        }
        //清理 TORT 表中的条目
        TORT[thread->PositionInTable] = 0;
        enable_interrupts();//恢复中断
        schedule();//触发调度
        SYSTEM_STOP(); // 理论上不会执行到这里
    }
    //如果线程在就绪队列中，从队列中移除
    if (ready_queue_head == thread) {
        // 是队列头部
        ready_queue_head = thread->next;
        if (!ready_queue_head) ready_queue_tail = NULL;
    } else {
        // 遍历队列查找并移除
        thread_t* curr = ready_queue_head;
        while (curr && curr->next != thread) {
            curr = curr->next;
        }
        if (curr) {
            curr->next = thread->next;
            if (thread == ready_queue_tail) {
                ready_queue_tail = curr;
            }
        }
    }
    //释放线程资源
    if (thread->kernel_stack) {
        pmm_free_pages(thread->kernel_stack, thread->stack_size / PAGE_SIZE);
        thread->kernel_stack = NULL;
    }
    //将 TCB 加入空闲链表
    if (num_of_FreeTCB < FREE_TCB_LIST_SIZE) {
        num_of_FreeTCB++;
        free_tcb_list[num_of_FreeTCB] = (uintptr_t)thread;
    }
    TORT[thread->PositionInTable] = 0;//清理 TORT 表中的条目
    enable_interrupts(); //重新开启中断
}


// 睡眠指定毫秒数
void ThreadSleepMS(uint64_t ms) {
    if (ms == 0) return;
    
    disable_interrupts();
    
    // 计算唤醒时间（基于时钟滴答）
    current_thread->wakeup_time = timer_ticks + (ms * PIT_CLOCK_FREQ) / 1000;
    current_thread->state = THREAD_BLOCKED;
    
    // 加入睡眠队列
    sleep_enqueue(current_thread);
    
    schedule(); // 触发调度
    
    enable_interrupts();
}

// 睡眠指定秒数
void ThreadSleepSecond(uint64_t s) {
    if (s == 0) return;
    
    disable_interrupts();
    
    // 计算唤醒时间
    current_thread->wakeup_time = timer_ticks + (s * PIT_CLOCK_FREQ);
    current_thread->state = THREAD_BLOCKED;
    
    // 加入睡眠队列
    sleep_enqueue(current_thread);
    
    schedule(); // 触发调度
    
    enable_interrupts();
}

// 唤醒到达时间的睡眠线程
void wake_sleeping_threads(void) {
    disable_interrupts();
    
    thread_t *current = sleep_queue_head;
    thread_t *prev = NULL;
    
    while (current) {
        if (timer_ticks >= current->wakeup_time) {
            // 时间到了，唤醒这个线程
            thread_t *to_wake = current;
            
            // 从睡眠队列中移除
            if (prev) {
                prev->next = current->next;
            } else {
                sleep_queue_head = current->next;
            }
            
            if (current == sleep_queue_tail) {
                sleep_queue_tail = prev;
            }
            
            current = current->next;
            
            // 重置连接指针
            to_wake->next = NULL;
            to_wake->state = THREAD_READY;
            
            // 加入就绪队列
            thread_enqueue(to_wake);
        } else {
            prev = current;
            current = current->next;
        }
    }
    
    enable_interrupts();
}

// 信号量相关函数
void sem_init(semaphore_t *sem, int value) {
    sem->count = value;
    sem->wait_queue_head = NULL;
    sem->wait_queue_tail = NULL;
}

static void sem_enqueue_wait(semaphore_t *sem, thread_t *thread) {
    thread->next = NULL;
    if (!sem->wait_queue_head) {
        sem->wait_queue_head = sem->wait_queue_tail = thread;
    } else {
        sem->wait_queue_tail->next = thread;
        sem->wait_queue_tail = thread;
    }
}

static thread_t *sem_dequeue_wait(semaphore_t *sem) {
    if (!sem->wait_queue_head) return NULL;
    thread_t *thread = sem->wait_queue_head;
    sem->wait_queue_head = thread->next;
    if (!sem->wait_queue_head) sem->wait_queue_tail = NULL;
    thread->next = NULL;
    return thread;
}

void sem_wait(semaphore_t *sem) {
    disable_interrupts();   // 关中断，保证原子性
    while (sem->count <= 0) {
        // 资源不足，当前线程进入等待队列并让出CPU
        current_thread->state = THREAD_BLOCKED;
        sem_enqueue_wait(sem, current_thread);
        schedule();          // 切换出去，返回时中断仍为关
        // 被唤醒后，重新检查信号量值（可能又被其他线程改变了，故用while循环）
    }
    // 成功获取资源
    sem->count--;
    enable_interrupts();
}

void sem_post(semaphore_t *sem) {
    disable_interrupts();
    sem->count++;  // 增加可用资源数
    
    // 唤醒一个等待线程（如果有）
    thread_t *waiter = sem_dequeue_wait(sem);
    if (waiter) {
        waiter->state = THREAD_READY;
        thread_enqueue(waiter);   // 放回就绪队列
    }
    enable_interrupts();
}

void PrintRunningThreads(){
    printf("ID \tStatus \tStack Size\n");
    printf("---------------------------------\n");
    printf("%d  \t%d      \t?KB\n",current_thread->id,current_thread->state);
    //遍历线程表
    for(int i = 0; i < TORT_SIZE; i++){
        if(TORT[i] == 0)continue;//如果为0,跳过
        thread_t *thread = (thread_t*)TORT[i];
        printf("%d  \t%d      \t%dKB\n",thread->id,thread->state,thread->stack_size/1024);
    }
}

void CleanAllThreads(){
    //遍历线程表
    for(int i = 0; i < TORT_SIZE; i++){
        if(TORT[i] == 0)continue;//如果无效,跳过
        thread_t *thread = (thread_t*)TORT[i];
        if((thread->id == 0) || (thread->id == 1))continue;//如果为主线程或IDLE，跳过
        KillThread(thread);
        out_ok("Thread ",0);
        printf("%d is over.\n",thread->id);
    }
}
