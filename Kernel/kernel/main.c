/*
    OS002 内核主程序

2026/3/6 Liu Chunyi

*/

#include <Tools.h>
#include <Draw.h>
#include <ACPI.h>
#include <Keyboard.h>
#include <Power.h>
#include <Print.h>
#include <Memory.h>
#include <Interrupt.h>
#include <Time.h>
#include <Task.h>
#include <Bitmap.h>

uint64_t CPU_Frequency = 0;
uint32_t *FrameBuffer = NULL;
uint16_t ScreenWidth = 0;
uint16_t ScreenHeigth = 0;
uint16_t PrintRow = 0;
uint16_t PrintLine = 0;
RSDP_DESCRIPTOR *RSDP = NULL;
uint8_t* font_data = NULL;
MEMORY_MAP* MemoryMap = NULL;

void SYSTEM_ERROR();
void ProgressBar(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint32_t color,uint8_t v);
void execute(char* cmd);
void UpdataCursor(uint32_t color);

void clock();
void _start(BOOT_SHARE* BootShare){
    disable_interrupts();
    FrameBuffer = (uint32_t*)BootShare->ScreenData->FrameBuffer;//设置缓冲区
    CPU_Frequency = BootShare->CPUF;//CPU基准频率
    RSDP = (RSDP_DESCRIPTOR*)BootShare->RSDP;//RSDP
    //屏幕分辨率
    ScreenHeigth = BootShare->ScreenData->SH;
    ScreenWidth = BootShare->ScreenData->SW;
    font_data = (uint8_t*)(uint64_t)BootShare->FONTADDR;//字体

    //自检
    if(!FrameBuffer) SYSTEM_ERROR();//检查帧缓冲区
    ProgressBar(0,ScreenHeigth - 20,ScreenWidth/2,20,COLOR_GREEN,5);//第一部分加载条
    if(!font_data) SYSTEM_ERROR();//检查字体
    //检查RSDP
    if((!RSDP)||(memcmp(RSDP->Signature,"RSD PTR ",8) != 0)){
        out_error("RSDP not Found!\n");
    }
    ProgressBar(ScreenWidth/2,ScreenHeigth - 20,ScreenWidth/2,20,COLOR_GREEN,5);//第二部分加载条
    asm("pause");
    uint64_t cpuf = Get_CPU_Frequency();
    if(cpuf != 0)CPU_Frequency = cpuf;
    execute("clear");
    MemoryMap = BootShare->MemoryMap;
    STATUS Status = InitPhysicalMemoryManager(MemoryMap);//初始化物理内存管理器
    if(Status == STATUS_SUCCESS){
        out_ok("Physical Memory Manager Initialized!\n",0);
    }else{
        out_error("Physical Memory Manager Initialization failed!");
        printf("Error Code:%x\n",Status);
    }
    Status = serial_init(SERIAL_COM1_PORT);
    if (Status != STATUS_SUCCESS) {
        out_error("Serial initialization failed!\n");
    }
    init_multitasking();

    out_ok("OS002 Kernel is running!\n",0);//打印正常运行信息
    serial_putstr(SERIAL_COM1_PORT, "OS002 Kernel is running!\n");
    char cmd[64] = "";
    int n = 0;
    //显示提示符
    print(">>>", COLOR_WHITE);
    UpdataCursor(COLOR_WHITE);
    while (1) {
        //读取输入循环
        while (1) {
            if(PrintLine > ScreenHeigth / 20){
                execute("clear");
                print(">>>", COLOR_WHITE);
                UpdataCursor(COLOR_WHITE);
                break;
            }
            char key = GetKey();//获取按键
            if (key == 0)break;
            if ((key == '\n') || (n >= 63)) {  //回车键
                UpdataCursor(COLOR_BLACK);
                printc('\n', COLOR_WHITE);  //打印换行
                cmd[n] = '\0';  //确保字符串以\0结尾
                if(strlen(cmd) > 0){
                    execute(cmd);
                }
                //显示提示符
                print(">>>", COLOR_WHITE);
                UpdataCursor(COLOR_WHITE);
                //清空命令缓冲区
                n = 0;
                cmd[0] = '\0';
                break;
            }
            //处理退格符
            if((key == '\b')&&(n > 0)){
                if(PrintRow > 0){
                    UpdataCursor(COLOR_BLACK);
                    PrintRow --;//光标回退
                    n --;//缓冲区“光标”回退
                    fillRect(PrintRow*10,PrintLine*18,10,16,COLOR_BLACK);//清除字符
                    cmd[n] = 0;//清除缓冲区
                    UpdataCursor(COLOR_WHITE);
                }
                break;
            }else if((key == '\b')&&(n == 0)){
                break;
            }
            UpdataCursor(COLOR_BLACK);
            // 显示字符并存入缓冲区
            printc(key, COLOR_WHITE);
            UpdataCursor(COLOR_WHITE);
            cmd[n] = key;
            n++;
        }
        asm("pause");
    }
    SYSTEM_STOP();

}

void SYSTEM_ERROR(){
    SYSTEM_STOP();
}
void ProgressBar(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint32_t color,uint8_t v){
    for(int _x = x;_x < w + x;_x ++){
        for(int _y = y;_y < h + y;_y ++){
            DrawPoint(_x,_y,color);
        }
        for(int i = 0;i < 100000 / v;i ++)asm("pause");
    }
}

static int task_y = 0;
void thread_a(void) {
    for(int i = 0;i < 200;i ++){
        DrawPoint(i,100,COLOR_RED);
        delay_ms(5);
    }
    delay_seconds(2);
}
void thread_b(void) {
    for(int i = 0;i < 200;i ++){
        DrawPoint(i,200,COLOR_GREEN);
        delay_ms(5);
    }
    delay_seconds(2);
}
static int shared_counter1 = 0;
static mutex_t counter_mutex = {0}; // 初始化互斥量
// 测试递归锁功能
void recursive_mutex_test_thread(void) {
    mutex_lock(&counter_mutex);
    printf("Thread %d got mutex first time\n", current_thread->id);
    // 尝试再次获取锁（递归锁测试）
    mutex_lock(&counter_mutex);
    printf("Thread %d owns mutex, can be used for recursive locking\n", current_thread->id);
    mutex_unlock(&counter_mutex);  // 递归锁解锁一次
    mutex_unlock(&counter_mutex);  // 递归锁解锁第二次
    printf("Thread %d released mutex\n", current_thread->id);
}

void thread_c(){
    while (1){
        DrawPoint(ScreenWidth/2,ScreenHeigth/2,COLOR_RED);
        delay_seconds(1);
        DrawPoint(ScreenWidth/2,ScreenHeigth/2,COLOR_GREEN);
        delay_seconds(1);
        DrawPoint(ScreenWidth/2,ScreenHeigth/2,COLOR_BLUE);
        delay_seconds(1);
    }
    
}
thread_t *testc;
void ThreadSleepTest(){
    printf("Sleeping for 2 seconds...");
    ThreadSleepSecond(2);
    printf("Woke up!\n");
}

void* mem;
static semaphore_t resource_sem;  // 示例信号量，限制最多2个线程访问资源
void thread_with_semaphore(void) {
    for (int i = 0; i < 5; i++) {
        sem_wait(&resource_sem);  // 请求资源
        printf("Thread %d: Using resource %d\n", current_thread->id, i);
        delay_ms(1000);  // 模拟使用资源
        printf("Thread %d: Done with resource %d\n", current_thread->id, i);
        sem_post(&resource_sem);  // 释放资源
    }
    printf("Thread %d finished\n", current_thread->id);
}

void execute(char* cmd){
    //hello命令
    if(strcmp(cmd,"hello") == 0){
        print("Hello World!\n",0xFFFFEE04);
    //清屏命令clear
    }else if(strcmp(cmd,"clear") == 0){
        UpdataCursor(COLOR_BLACK);
        fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_BLACK);//清屏
        //光标归位
        PrintLine = 0;
        PrintRow = 0;
    //重启
    }else if(strcmp(cmd,"reboot") == 0){
        delay_ms(100);
        fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_BLACK);//清屏
        //光标归位
        PrintLine = 0;
        PrintRow = 0;
        CleanAllThreads();
        print("[POWER]Rebooting...\n",COLOR_CYAN);
        delay_seconds(1);
        SYSTEM_REBOOT();
    //关机
    }else if(strcmp(cmd,"shutdown") == 0){
        delay_ms(100);
        fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_BLACK);//清屏
        //光标归位
        PrintLine = 0;
        PrintRow = 0;
        CleanAllThreads();
        print("[POWER]Shutting down...\n",COLOR_CYAN);
        delay_seconds(1);
        SYSTEM_SHUTDOWN();
    //RSDP地址
    }else if(strcmp(cmd,"rsdp") == 0){
        //解析版本
        char* ver;
        if(RSDP->Revision == 0){
            ver = "V1.0";
        }else{
            ver = "V2.0+";
        }
        printf("RSDP at %x\nACPI Version:%s\n",(uint64_t)RSDP,ver);
    //XSDT地址
    }else if(strcmp(cmd,"xsdt") == 0){
        XSDT_DESCRIPTOR* XSDT = (XSDT_DESCRIPTOR*)RSDP->XsdtAddress;
        if(memcmp(XSDT->Signature,"XSDT",4) == 0){
            printf("XSDT at %x\n",(uint64_t)RSDP->XsdtAddress);
        }
    //FADT地址
    }else if(strcmp(cmd,"findFADT") == 0){
        ACPI_SDT_HEADER* FADT = find_FADT();
        if(!FADT){
            FADT = find_FADT_in_RSDT();
        }
        if(!FADT){
            out_error("FADT not found\n");
        }else{
            printf("FADT at:%x\n",(uint64_t)FADT);
        }
    //当前时间
    }else if(strcmp(cmd,"time") == 0){
        struct rtc_time time;
        get_rtc_time(&time);//获取时间
        printf("20%d/%d/%d\n%d:%d:%d\n",time.year,time.month,time.day,time.hour,time.minute,time.second);//显示时间
    //时钟
    }else if(strcmp(cmd,"clock") == 0){
        clock();
        fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_BLACK);//清屏
        //光标归位
        PrintLine = 0;
        PrintRow = 0;
        delay_ms(800);
    //系统状态
    } else if(strcmp(cmd,"status") == 0){
        uint64_t cpuf = Get_CPU_Frequency();//获取CPU频率
        //更新频率
        if(cpuf != 0){
            CPU_Frequency = cpuf;
        }else{
            out_error("Get CPU frequency failed!\n");
        }
        //显示信息
        printf("CPU Frequency:   %d Hz\n", CPU_Frequency);
        printf("Screen:          %dx%d\n", ScreenWidth, ScreenHeigth);
        printf("FrameBuffer:     %x\n", (uint64_t)FrameBuffer);
        printf("RSDP:            %p\n", (uint64_t)RSDP);
        printf("Font:            %p\n", (uint64_t)font_data);
        printf("MemoryMap:       %p\n", (uint64_t)MemoryMap);
        printf("MemoryTotalSize: %d MB\n", GetMemoryTotalSize(MemoryMap) / 1024 / 1024);
    //第6个内存描述符信息
    }else if(strcmp(cmd,"MMD") == 0){
        MEMORY_DESCRIPTOR* MemoryDescriptor = AnalysisMemoryMap(MemoryMap,0);//获取描述符地址
        if(!MemoryDescriptor){
            out_error("AnalysisMemoryDescriptor failed\n");//如果无效则打印错误信息
        }else{
            PrintMemoryDescriptor(MemoryDescriptor);//否则打印描述符信息
        }
    //打印3个可用的内存
    }else if(strcmp(cmd,"AvaMD") == 0){
        PrintAvailableMemoryDescriptor(MemoryMap,10);
    //查找并打印最大的内存块的描述符
    }else if(strcmp(cmd,"MaxMD") == 0){
        MEMORY_DESCRIPTOR* MemoryDescriptor = FindLargestAvailableBlock(MemoryMap);
        if(!MemoryDescriptor){
            out_error("AnalysisMemoryDescriptor failed\n");//如果无效则打印错误信息
        }else{
            PrintMemoryDescriptor(MemoryDescriptor);//否则打印描述符信息
        }
    //开辟内存空间
    }else if(strcmp(cmd,"AcMM") == 0){
        mem = pmm_alloc_pages(2);//开辟2页的空间
        if(!mem){
            out_error("Failed to AllocateMemory\n");
        }else{
            printf("8KB Memory at %x\n",(uint64_t)mem);
        }
    //回收
    }else if(strcmp(cmd,"FreeMM") == 0){
        if(!mem){
            out_error("Failed to FreeMemory\n");
        }else{
            pmm_free_pages(mem,2);
        }
    //中断状态
    }else if(strcmp(cmd,"IntStatus") == 0){
        printf("%s\n",(are_interrupts_enabled() == 1) ? "Enabled" : "Disabled");
    //中断使能
    }else if(strcmp(cmd,"IntEnable") == 0){
        enable_interrupts();
    //中断禁止
    }else if(strcmp(cmd,"IntDisable") == 0){
        disable_interrupts();
    //初始化中断
    }else if(strcmp(cmd,"InitINT") == 0){
        Init_IDT();
    //键盘中断测试
    }else if(strcmp(cmd,"KeyBoard") == 0){
        irq_enable(IRQ1);
        while(1){
            uint8_t Key = IRQKey;
            if(Key != 0){
                printf("%x\n",Key);
                delay_seconds(1);
            }
            if(PrintLine > (ScreenHeigth / 16)){
                fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_BLACK);
            }
        }
    //时钟中断测试
    }else if(strcmp(cmd,"ticks") == 0){
        printf("%d\n",timer_ticks);
        delay_seconds(1);
        printf("%d\n",timer_ticks);
    //蜂鸣器测试
    }else if(strcmp(cmd,"beep") == 0){
        beep(440,1000);
    //除零测试
    }else if(strcmp(cmd,"DE") == 0){
        int a = 2;
        int b = 0;
        printf("2/0=%d\n",a/b);
    //创建线程测试
    } else if(strcmp(cmd,"createThread") == 0) {
        thread_t *t = create_kernel_thread(thread_a, 4096);
        thread_t *t1 = create_kernel_thread(thread_b, 4096);
        if (t&&t1) {
            printf("Thread created: ID=%d, stack at %x\n", t->id, (uint64_t)t->kernel_stack);
            printf("Thread created: ID=%d, stack at %x\n", t1->id, (uint64_t)t1->kernel_stack);
        } else {
            out_error("Thread creation failed!\n");
        }
    //互斥锁测试
    } else if(strcmp(cmd, "testmutex") == 0) {
        mutex_init(&counter_mutex);
        create_kernel_thread(recursive_mutex_test_thread, 4096);
        create_kernel_thread(recursive_mutex_test_thread, 4096);
    //打印正在运行的线程
    } else if(strcmp(cmd, "rt") == 0) {
        PrintRunningThreads();
    //创建测试线程
    } else if(strcmp(cmd, "starttest") == 0) {
        testc = create_kernel_thread(thread_c,4096);
    //结束测试线程
    } else if(strcmp(cmd, "killtest") == 0) {
        KillThread(testc);
    //睡眠测试
    }else if(strcmp(cmd, "sleeptest") == 0) {
        create_kernel_thread(ThreadSleepTest,4096);
    //发送信号测试
    }else if(strcmp(cmd, "sempost") == 0) {
        sem_post(&resource_sem);
    //等待测试
    }else if(strcmp(cmd, "semwait") == 0) {
        sem_wait(&resource_sem);
        printf("Got semaphore!\n");
    //初始化信号量
    }else if(strcmp(cmd, "seminit") == 0) {
        int val = 2;  // 最多允许2个线程同时访问资源
        sem_init(&resource_sem, val);
        printf("Semaphore initialized with value %d\n", val);
    //信号量测试
    }else if(strcmp(cmd, "semtest") == 0) {
        sem_init(&resource_sem, 2);  // 初始化信号量，最多2个线程可同时访问
        create_kernel_thread(thread_with_semaphore, 4096);
        create_kernel_thread(thread_with_semaphore, 4096);
        create_kernel_thread(thread_with_semaphore, 4096);  // 第三个线程需要等待
    //位图测试
    }else if(strcmp(cmd, "bitmaptest") == 0) {
        uint8_t bits[8];
        bitmap_t bm;
        BitmapInit(&bm,bits,64,0);
        BitmapAllocBits(&bm,0,10);
        BitmapSetBits(&bm,30,4,1);
        uint8_t val = BitmapGetBit(&bm,4);
        printf("val=%d\n",val);
    }else{
        out_waring("Command not found!\n");
    }
}

void UpdataCursor(uint32_t color){
    for(int i = 0;i < 16;i ++){
        DrawPoint(PrintRow * 10,PrintLine * 18+i,color);
    }
}

void clock(){
    while(1){
        fillRect(0,0,ScreenWidth,ScreenHeigth,COLOR_GREY);//清屏
        //设置光标
        PrintLine = 2;
        PrintRow = 2;
        struct rtc_time time;
        get_rtc_time(&time);//获取时间
        printf("20%d/%d/%d\n%d:%d:%d\n",time.year,time.month,time.day,time.hour,time.minute,time.second);//显示
        //如果按下E键则推出
        if((GetKey_NoBlock() == 'e')||(GetKey_NoBlock() == 'E')){
            break;
        }
        delay_ms(600);
    }
}
