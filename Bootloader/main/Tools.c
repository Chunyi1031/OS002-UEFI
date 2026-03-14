#include "Tools.h"

// 全局系统表变量
EFI_SYSTEM_TABLE *GlobalST = NULL;
EFI_HANDLE GlobalImageHandle = NULL;
EFI_BOOT_SERVICES *GlobalBS = NULL;
EFI_GRAPHICS_OUTPUT_PROTOCOL *GlobalGOP = NULL;

//单位换算
int ms_to_us(const int ms) { return (ms * 1000); }
int s_to_us(const int s) { return (s * 1000000); }
int B_to_MB(const int b) { return (b / 1048576); }

//停机
void STOP(){
    //死循环
    while (1){
        __asm__("hlt");//降低CPU利用率
    }
    
}
//延迟
void delay(const int us) {
    uefi_call_wrapper(GlobalST->BootServices->Stall, 1, us);
}

//设置屏幕颜色
void SetColor(const INTN textc, const INTN bgc) {
    uefi_call_wrapper(GlobalST->ConOut->SetAttribute, 2, GlobalST->ConOut, textc | bgc);
}

//清屏
void ClearScreen() {
    uefi_call_wrapper(GlobalST->ConOut->ClearScreen, 1, GlobalST->ConOut);
}

//换行打印
void println(const char *s) {
    Print(L"%a\n", s);
}
//设置光标位置
void SetCursorPosition(UINTN Column, UINTN Row) {
    uefi_call_wrapper(GlobalST->ConOut->SetCursorPosition, 3, GlobalST->ConOut, Column, Row);
}

EFI_STATUS
ReadFileToBuffer(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable,
    IN CHAR16 *FilePath,
    OUT VOID **Buffer,
    OUT UINTN *BufferSize,
    IN UINTN MaxBytes
)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *RootVolume = NULL;
    EFI_FILE_PROTOCOL *FileHandle = NULL;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    UINTN InfoSize;
    UINT64 FileSize;
    UINTN BytesToRead;
    UINTN BytesRead;
    
    if (!SystemTable || !FilePath || !Buffer || !BufferSize) {
        return EFI_INVALID_PARAMETER;
    }
    
    *Buffer = NULL;
    *BufferSize = 0;
    
    Status = uefi_call_wrapper(
        SystemTable->BootServices->HandleProtocol,
        3,
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID**)&LoadedImage
    );
    
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    Status = uefi_call_wrapper(
        SystemTable->BootServices->HandleProtocol,
        3,
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID**)&FileSystem
    );
    
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    Status = uefi_call_wrapper(
        FileSystem->OpenVolume,
        2,
        FileSystem,
        &RootVolume
    );
    
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    Status = uefi_call_wrapper(
        RootVolume->Open,
        5,
        RootVolume,
        &FileHandle,
        FilePath,
        EFI_FILE_MODE_READ,
        0
    );
    
    if (EFI_ERROR(Status)) {
        uefi_call_wrapper(RootVolume->Close, 1, RootVolume);
        return Status;
    }
    
    InfoSize = sizeof(EFI_FILE_INFO) + 128;
    Status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePool,
        3,
        EfiLoaderData,
        InfoSize,
        (VOID**)&FileInfo
    );
    
    if (EFI_ERROR(Status)) {
        uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
        uefi_call_wrapper(RootVolume->Close, 1, RootVolume);
        return Status;
    }
    
    Status = uefi_call_wrapper(
        FileHandle->GetInfo,
        4,
        FileHandle,
        &gEfiFileInfoGuid,
        &InfoSize,
        FileInfo
    );
    
    if (EFI_ERROR(Status)) {
        uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, FileInfo);
        uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
        uefi_call_wrapper(RootVolume->Close, 1, RootVolume);
        return Status;
    }
    
    FileSize = FileInfo->FileSize;
    BytesToRead = (UINTN)FileSize;
    
    if (MaxBytes > 0 && MaxBytes < FileSize) {
        BytesToRead = MaxBytes;
    }
    
    Status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePool,
        3,
        EfiLoaderData,
        BytesToRead,
        Buffer
    );
    
    if (EFI_ERROR(Status)) {
        uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, FileInfo);
        uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
        uefi_call_wrapper(RootVolume->Close, 1, RootVolume);
        return Status;
    }
    
    BytesRead = BytesToRead;
    Status = uefi_call_wrapper(
        FileHandle->Read,
        3,
        FileHandle,
        &BytesRead,
        *Buffer
    );
    
    uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, FileInfo);
    uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
    uefi_call_wrapper(RootVolume->Close, 1, RootVolume);
    
    if (EFI_ERROR(Status)) {
        uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, *Buffer);
        *Buffer = NULL;
        return Status;
    }
    
    *BufferSize = BytesRead;
    return EFI_SUCCESS;
}

CHAR16* HexToChar(VOID *fileData,UINTN Size){
    UINTN i;
    CHAR16 *wideStr = NULL;
    UINT8 *data = (UINT8*)fileData;
    if (!fileData || Size == 0) {
        return 0;
    }
    // 分配宽字符串缓冲区（每个字符需要2字节）
    wideStr = AllocatePool((Size + 1) * sizeof(CHAR16));
    if (!wideStr) {
        return 0;
    }
    // 将ASCII字符转换为宽字符（CHAR16）
    for (i = 0; i < Size; i++) {
        // 只转换可打印的ASCII字符（32-126）
        // 非打印字符显示为点
        if (data[i] >= 32 && data[i] <= 126) {
            wideStr[i] = data[i];  // ASCII转宽字符（低字节）
        } else {
            wideStr[i] = L'.';     // 不可打印字符显示为点
        }
    }
    // 添加终止符
    wideStr[Size] = L'\0';
    return wideStr;
}

UINTN Memcmp(const void* s1, const void* s2, UINT64 n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    for (UINT64 i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return (int)p1[i] - (int)p2[i];  // 返回差值
        }
    }
    return 0;  // 完全相同
}

VOID PrintFileContentAsString(VOID *fileData, UINTN fileSize){
    // 打印字符串
    Print(L"%s\n", HexToChar(fileData,fileSize));
    // 释放内存
    FreePool(fileData);
}
//设置屏幕分辨率模式
void SetScreenMode(const UINTN mode){
    uefi_call_wrapper(GlobalGOP->SetMode, 2, GlobalGOP, mode);
}
//设置屏幕分辨率
void SetScreenSize(UINT32 wide, UINT32 high) {
    UINTN i;
    for (i = 0; i < GlobalGOP->Mode->MaxMode; i++) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
        UINTN size;
        if (!EFI_ERROR(uefi_call_wrapper(GlobalGOP->QueryMode, 4, GlobalGOP, i, &size, &info))) {
            if((info->HorizontalResolution == wide) && (info->VerticalResolution == high)) {
                SetScreenMode(i);
                uefi_call_wrapper(BS->FreePool, 1, info);
                break;
            }
            uefi_call_wrapper(BS->FreePool, 1, info);
        }
    }
}
//初始化图形显示
EFI_STATUS InitGraphics() {
    EFI_STATUS status;
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&GlobalGOP);
    if (EFI_ERROR(status)) {
        return status;
    }
    return EFI_SUCCESS;
}
void SetPixel(UINTN x, UINTN y, PixelColor color) {
    if (!GlobalGOP) return;
    UINTN screen_width, screen_height;
    // 获取当前分辨率
    screen_width = GlobalGOP->Mode->Info->HorizontalResolution;
    screen_height = GlobalGOP->Mode->Info->VerticalResolution;
    // 检查坐标是否在屏幕范围内
    if (x >= screen_width || y >= screen_height) {
        return;
    }
    UINTN pitch = GlobalGOP->Mode->Info->PixelsPerScanLine;// 获取像素每扫描线数
    PixelColor *framebuffer = (PixelColor*)GlobalGOP->Mode->FrameBufferBase;//获取帧缓冲区地址
    framebuffer[y * pitch + x] = color;//计算像素位置并设置颜色
}

void ClearScreenGraphics(EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop) {
    UINTN ScreenWidth = Gop->Mode->Info->HorizontalResolution;
    UINTN ScreenHeight = Gop->Mode->Info->VerticalResolution;
    UINT32 *FrameBuffer = (UINT32*)Gop->Mode->FrameBufferBase;
    // 填充黑色
    for (UINTN y = 0; y < ScreenHeight; y++) {
        for (UINTN x = 0; x < ScreenWidth; x++) {
            FrameBuffer[y * ScreenWidth + x] = 0x00000000;  // ARGB黑色
        }
    }
}

UINT64 GetCPUFrequency() {
    // 延迟10ms测量TSC
    UINT64 start, end;
    
    asm volatile ("rdtsc" : "=A" (start));
    gBS->Stall(10000);  // 10ms延迟
    asm volatile ("rdtsc" : "=A" (end));
    
    UINT64 freq = (end - start) * 100;
    
    return freq;
}

EFI_STATUS Bmp(VOID* BmpData,UINT16 PosX,UINT16 PosY){
    EFI_STATUS Status;
    // 4. 解析BMP并显示（核心部分）
    BMP_FILE_HEADER *FileHeader = (BMP_FILE_HEADER*)BmpData;
    BMP_INFO_HEADER *InfoHeader = (BMP_INFO_HEADER*)(FileHeader + 1);
    
    // 验证BMP格式
    if (FileHeader->Type != 0x4D42) { // "BM"
        Print(L"Error: Not a valid BMP file\n");
        uefi_call_wrapper(gBS->FreePool, 1, BmpData);
        return EFI_UNSUPPORTED;
    }
    
    // 只支持24位不压缩BMP
    if (InfoHeader->BitCount != 24 || InfoHeader->Compression != 0) {
        Print(L"Error: Only 24-bit uncompressed BMP supported\n");
        uefi_call_wrapper(gBS->FreePool, 1, BmpData);
        return EFI_UNSUPPORTED;
    }
    
    // 计算行字节数（BMP每行字节数必须是4的倍数）
    UINT32 Width = InfoHeader->Width;
    UINT32 Height = InfoHeader->Height;
    UINT32 RowBytes = ((Width * 3 + 3) & ~3); // 每行字节数（4字节对齐）
    
    // 5. 分配BLT缓冲区并转换格式
    UINTN BltBufferSize = Width * Height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer;
    
    Status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderData,
                               BltBufferSize, (VOID**)&BltBuffer);
    if (EFI_ERROR(Status)) {
        uefi_call_wrapper(gBS->FreePool, 1, BmpData);
        return Status;
    }
    
    // 像素数据起始位置
    UINT8 *PixelData = (UINT8*)BmpData + FileHeader->OffBits;
    
    // 转换BGR到BLT格式（BMP是BGR顺序，从下往上存储）
    for (UINT32 y = 0; y < Height; y++) {
        UINT8 *SrcRow = PixelData + (Height - 1 - y) * RowBytes; // BMP从下往上存储
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL *DstRow = &BltBuffer[y * Width];
        
        for (UINT32 x = 0; x < Width; x++) {
            UINT8 *Pixel = SrcRow + x * 3;
            DstRow[x].Blue = Pixel[0];
            DstRow[x].Green = Pixel[1];
            DstRow[x].Red = Pixel[2];
            DstRow[x].Reserved = 0xFF; // Alpha通道设为不透明
        }
    }
    
    // 6. 显示到屏幕
    Status = uefi_call_wrapper(GlobalGOP->Blt, 10, GlobalGOP, BltBuffer,
                               EfiBltBufferToVideo, 0, 0, PosX, PosY,
                               Width, Height, 0);
    
    // 7. 清理
    uefi_call_wrapper(gBS->FreePool, 1, BltBuffer);
    uefi_call_wrapper(gBS->FreePool, 1, BmpData);
}

VOID* GetRSDP(EFI_SYSTEM_TABLE *SystemTable) {
    // 定义 ACPI GUID
    EFI_GUID Acpi20Guid = ACPI_20_TABLE_GUID;
    EFI_GUID Acpi10Guid = ACPI_TABLE_GUID;
    // 遍历系统配置表
    for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++) {
        // 先找 ACPI 2.0
        if (CompareGuid(&SystemTable->ConfigurationTable[i].VendorGuid, &Acpi20Guid) == 0) {
            return SystemTable->ConfigurationTable[i].VendorTable;
        }
    }
    // 如果没找到 2.0，找 1.0
    for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++) {
        if (CompareGuid(&SystemTable->ConfigurationTable[i].VendorGuid, &Acpi10Guid) == 0) {
            return SystemTable->ConfigurationTable[i].VendorTable;
        }
    }
    return NULL;
}