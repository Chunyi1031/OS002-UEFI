/*
    OS002 UEFI Bootloader

2026/3/6 Liu Chunyi

*/

#include <efi.h>
#include <efilib.h>
#include <Tools.h>

#define SWidth  1920 //屏幕宽
#define SHieght 1080 //屏幕高

#define Kernel_Path L"\\SYS\\kernel"        //内核路径
#define Fonts_Path  L"\\SYS\\Fonts10x16.bin"//字体路径
#define Logo_Path   L"\\SYS\\Logo.bmp"      //图表路径

typedef struct KNL_FILE_HEADER{
    CHAR8 Signature[5];//文件标识
    UINT32 ReVersion;//文件格式版本
    UINT8 Reserved[7];//保留
    CHAR8 KernelName[5];//内核名称
    CHAR8 KernelVersion[5];//内核版本
    UINT8 Reserved2[6];//保留
} __attribute__((packed)) KNL_FILE_HEADER;

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
    //设置全局变量
    GlobalST = SystemTable;
    GlobalImageHandle = ImageHandle;
    GlobalBS = SystemTable->BootServices;
    //初始化库
    InitializeLib(ImageHandle, SystemTable);
    
    #ifdef debug
    println("Booting...");
    #endif
    //初始化Gop
    EFI_STATUS Status = InitGraphics();
    if(EFI_ERROR(Status)){
        SetColor(EFI_RED,EFI_BACKGROUND_BLACK);
        Print(L"[ERROR]InitGraphics failed:%r\n",Status);
        delay(s_to_us(30));//延迟30秒以便于查看错误信息
        uefi_call_wrapper(SystemTable->RuntimeServices->ResetSystem, 4, EfiResetWarm, EFI_SUCCESS, 0, NULL);//重启
        return Status;//如果重启失败，停止运行
    }
    delay(ms_to_us(100));
    SetScreenSize(SWidth,SHieght);//设置屏幕分辨率
    delay(ms_to_us(20));
    // 获取当前分辨率
    UINTN Screen_Width = GlobalGOP->Mode->Info->HorizontalResolution;
    UINTN Screen_Height = GlobalGOP->Mode->Info->VerticalResolution;
    #ifdef debug
    Print(L"[OK]Screen Size:%dx%d\n",Screen_Width,Screen_Height);
    #endif
    #ifndef NotLogo
    //读取图标
    VOID* PictureData = NULL;
    UINTN PictureSize = 0;
    Status = ReadFileToBuffer(ImageHandle,SystemTable,Logo_Path,&PictureData,&PictureSize,0);
    if(EFI_ERROR(Status)){
        #ifdef debug
        SetColor(EFI_YELLOW,EFI_BACKGROUND_BLACK);
        Print(L"[WARING]Read picture failed:%r\n",Status);
        delay(s_to_us(5));
        SetColor(EFI_WHITE,EFI_BACKGROUND_BLACK);
        #endif
    }else{
        //显示图标
        Status = Bmp(PictureData,((Screen_Width/2) - 128),((Screen_Height/2) - 64));
        if(EFI_ERROR(Status)){
            #ifdef debug
            SetColor(EFI_YELLOW,EFI_BACKGROUND_BLACK);
            Print(L"[WARING]The image failed to load:%r\n",Status);
            delay(s_to_us(5));
            SetColor(EFI_WHITE,EFI_BACKGROUND_BLACK);
            #endif
        }
    }
    #endif

    //加载内核
    VOID* Kernel_Buffer = NULL;
    UINTN Kernel_Size = 0;
    Status = ReadFileToBuffer(ImageHandle,SystemTable,Kernel_Path,&Kernel_Buffer,&Kernel_Size,0);
    //检查加载情况
    if(EFI_ERROR(Status)){
        SetColor(EFI_RED,EFI_BACKGROUND_BLACK);
        Print(L"[ERROR]The kernel faild to load:%r\n",Status);
        delay(s_to_us(30));//延迟30秒以便于查看错误信息
        uefi_call_wrapper(SystemTable->RuntimeServices->ResetSystem, 4, EfiResetWarm, EFI_SUCCESS, 0, NULL);//重启
        return Status;//如果重启失败，停止运行
    }
    //检查文件头
    KNL_FILE_HEADER* Header = (KNL_FILE_HEADER*)Kernel_Buffer;
    if((Memcmp(Header->Signature,"KNL64",5) != 0) || (Header->ReVersion != 0)){
        SetColor(EFI_RED,EFI_BACKGROUND_BLACK);
        println("[ERROR]Kernel damage!\n");
        delay(s_to_us(30));//延迟30秒以便于查看错误信息
        uefi_call_wrapper(SystemTable->RuntimeServices->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);//重启
        return Status;//如果重启失败，停止运行
    }
    #ifdef debug
    SetColor(EFI_GREEN,EFI_BACKGROUND_BLACK);
    Print(L"[SUCESS]Kernel load at:0x%lx,Size:%dB\n",Kernel_Buffer,Kernel_Size);
    #endif
    //加载字体
    VOID* Font_Buffer = NULL;
    UINTN Font_Size = 0;
    Status = ReadFileToBuffer(ImageHandle,SystemTable,Fonts_Path,&Font_Buffer,&Font_Size,0);
    //检查加载情况
    if(EFI_ERROR(Status)){
        #ifdef debug
        SetColor(EFI_YELLOW,EFI_BACKGROUND_BLACK);
        Print(L"[WARING]Read fonts failed:%r\n",Status);
        delay(s_to_us(5));
        SetColor(EFI_GREEN,EFI_BACKGROUND_BLACK);
        #endif
    }else{
        #ifdef debug
        Print(L"[SUCESS]Fonts load at:0x%lx\n",Font_Buffer);
        SetColor(EFI_WHITE,EFI_BACKGROUND_BLACK);
        #endif
    }
    delay(ms_to_us(50));

    //获取映射
    SHARE_KERNEL Share;
    SCREEN_DATA ScreenData;
    MEMORY_MAP MemoryMap;
    ScreenData.FrameBuffer = GlobalGOP->Mode->FrameBufferBase;//帧缓冲区
    ScreenData.SH = Screen_Height;//屏幕高
    ScreenData.SW = Screen_Width;//屏幕宽
    //获取 RSDP
    VOID *Rsdp = GetRSDP(SystemTable);
    if (Rsdp != NULL) {
        #ifdef debug
        SetColor(EFI_GREEN,EFI_BACKGROUND_BLACK);
        Print(L"[SUCESS]RSDP found at: 0x%lx\n", Rsdp);
        SetColor(EFI_WHITE,EFI_BACKGROUND_BLACK);
        delay(ms_to_us(50));
        #endif
    }else{
        SetColor(EFI_RED,EFI_BACKGROUND_BLACK);
        println("[ERROR]Rsdp not found!\n");
        delay(s_to_us(30));//延迟30秒以便于查看错误信息
        uefi_call_wrapper(SystemTable->RuntimeServices->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);//重启
        return Status;//如果重启失败，停止运行
    }
    Share.CPUF = 2000000000;
    Share.ScreenData = &ScreenData;
    Share.RSDP = Rsdp;
    Share.FONTADDR = (UINT32)(UINTN)Font_Buffer;
    //分配专用栈空间
    VOID *StackBase;
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, 0x80000, &StackBase);
    VOID *StackPointer = (VOID*)((UINTN)StackBase + 0x80000);  //栈顶
    StackPointer = (VOID*)((UINTN)StackPointer & ~0xF);      //16字节对齐
    #ifdef debug
    SetColor(EFI_GREEN,EFI_BACKGROUND_BLACK);
    Print(L"[SUCESS]Stack allocated at:0x%lx\n",StackPointer);
    SetColor(EFI_WHITE,EFI_BACKGROUND_BLACK);
    #endif
    MemoryMap.StackAddr = StackPointer;//栈底
    MemoryMap.MapSize = 8192;
    MemoryMap.Buffer = NULL;
    UINTN MapKey = 0;
    MemoryMap.DescriptorSize = 0;
    MemoryMap.DescriptorVersion = 0;
    Status = uefi_call_wrapper(gBS->AllocatePool,3,EfiLoaderData, MemoryMap.MapSize, &MemoryMap.Buffer);
    if(!EFI_ERROR(Status)){
        Status = uefi_call_wrapper(gBS->GetMemoryMap,5,
            &MemoryMap.MapSize,
            MemoryMap.Buffer,
            &MapKey,
            &MemoryMap.DescriptorSize, 
            &MemoryMap.DescriptorVersion);
        Share.MemoryMap = &MemoryMap;
        Status = uefi_call_wrapper(gBS->ExitBootServices,2,ImageHandle, MapKey);
        if(EFI_ERROR(Status)){
            #ifdef debug
            //黄字打印警告
            SetColor(EFI_YELLOW,EFI_BACKGROUND_BLACK);
            Print(L"[WRN]Exit boot services failed:%r",Status);
            delay(s_to_us(2));
            #endif
        }
    }

    //跳转
    asm volatile("mov %0, %%rsp\n\tcli": :"r"(StackPointer));//切换内核栈
    void (*OS002_Kernel)(SHARE_KERNEL*) = Kernel_Buffer + sizeof(KNL_FILE_HEADER);//偏移0x20以跳过文件头
    OS002_Kernel(&Share);//运行内核主函数

    STOP();//如国返回，停机

    return EFI_SUCCESS;
}
