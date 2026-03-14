/*
    Tools.h Tools.c
 引导程序基础库

2026/3/6 Liu Chunyi

*/

#ifndef TOOLS_H
#define TOOLS_H

#include <efi.h>
#include <efilib.h>

// 全局系统表变量
extern EFI_SYSTEM_TABLE *GlobalST;
extern EFI_HANDLE GlobalImageHandle;
extern EFI_BOOT_SERVICES *GlobalBS;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL *GlobalGOP;

typedef struct {
    UINT8 Blue;
    UINT8 Green;
    UINT8 Red;
    UINT8 Reserved;
} PixelColor;

typedef struct{
    UINT64 FrameBuffer;
    UINT16 SW;
    UINT16 SH;
}SCREEN_DATA;

typedef struct{
    UINTN MapSize;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    VOID* StackAddr;
    VOID* Buffer;
}MEMORY_MAP;

typedef struct {
    SCREEN_DATA* ScreenData;
    UINT64 CPUF;
    UINT32 FONTADDR;
    VOID *RSDP;
    MEMORY_MAP* MemoryMap;
} SHARE_KERNEL;

#pragma pack(1)
typedef struct {
    UINT16  Type;          // 文件类型，必须是"BM" (0x4D42)
    UINT32  Size;          // 文件大小
    UINT16  Reserved1;     // 保留
    UINT16  Reserved2;     // 保留
    UINT32  OffBits;       // 像素数据偏移量
} BMP_FILE_HEADER;

typedef struct {
    UINT32  Size;          // 信息头大小
    UINT32  Width;         // 图像宽度(像素)
    UINT32  Height;        // 图像高度(像素)
    UINT16  Planes;        // 必须为1
    UINT16  BitCount;      // 每像素位数(1,4,8,24)
    UINT32  Compression;   // 压缩类型(0=不压缩)
    UINT32  SizeImage;     // 图像数据大小
    UINT32  XPelsPerMeter; // 水平分辨率
    UINT32  YPelsPerMeter; // 垂直分辨率
    UINT32  ClrUsed;       // 使用的颜色数
    UINT32  ClrImportant;  // 重要颜色数
} BMP_INFO_HEADER;
#pragma pack()

//单位换算
int ms_to_us(const int ms);
int s_to_us(const int s);
int B_to_MB(const int b);

void STOP();//停机
void delay(const int us);//延迟
void SetColor(const INTN textc, const INTN bgc);//设置字体颜色（字，背景）
void ClearScreen();//清屏
UINTN Memcmp(const void* s1, const void* s2, UINT64 n);//内存比较
void println(const char *s);//换行打印字符串
void SetCursorPosition(UINTN Column, UINTN Row);//设置光标位置
EFI_STATUS ReadFileToBuffer(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable,IN CHAR16 *FilePath,OUT VOID **Buffer,OUT UINTN *BufferSize,IN UINTN MaxBytes);//读取文件
void PrintFileContentAsString(VOID *fileData, UINTN fileSize);//打印16进制文件数据
CHAR16* HexToChar(VOID *fileData,UINTN Size);//将16进制转化为宽体字符串
void SetScreenMode(const UINTN mode);//设置屏幕分辨率模式
void SetScreenSize(UINT32 wide, UINT32 high);//设置屏幕分辨率
EFI_STATUS InitGraphics();//初始化图型模式
void SetPixel(UINTN x, UINTN y, PixelColor color);//绘制点
void ClearScreenGraphics(EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop);//图型模式清屏
UINT64 GetCPUFrequency();
EFI_STATUS Bmp(VOID* BmpData,UINT16 PosX,UINT16 PosY);//显示图片
VOID* GetRSDP(EFI_SYSTEM_TABLE *SystemTable);//获取ACPI起始地址

#endif