#include <Print.h>

void DrawString1(char *s,int x, int y, uint32_t color){
    int startX = x;//记录起始X坐标
    int currentX = x;
    int currentY = y;
    int charWidth = 10;//宽
    int charHeight = 16;//高
    int lineSpacing = 2;//行间距
    size_t size = strlen(s);
    for (int i = 0; i < size; i++) {
        char c = s[i];
        //处理换行符
        if (c == '\n') {
            currentX = startX;//X坐标回到起始位置
            currentY += charHeight + lineSpacing;//Y坐标下移一行
            PrintLine++;
            PrintRow = 0;
            continue;
        }
        //处理制表符
        if (c == '\t') {
            //打印tabSize个空格
            for (int j = 0; j < tabSize; j++) {
                //检查是否需要自动换行
                if (currentX + charWidth > ScreenWidth) {
                    currentX = startX;//X坐标回到起始位置
                    currentY += charHeight + lineSpacing;//Y坐标下移一行
                    PrintLine++;
                    PrintRow = 0;
                }
                
                //打印一个空格
                DrawChar(' ', currentX, currentY, color);//绘制空格字符
                currentX += charWidth;//更新X坐标
            }
            PrintRow += tabSize;
            continue;
        }
        //检查是否需要自动换行（如果字符串太长超出屏幕）
        if (currentX + charWidth > ScreenWidth) {
            currentX = startX;//X坐标回到起始位置
            currentY += charHeight + lineSpacing;//Y坐标下移一行
            PrintLine++;
            PrintRow = 0;
        }
        DrawChar(c, currentX, currentY, color);//绘制字符
        currentX += charWidth;//更新X坐标，准备绘制下一个字符
        PrintRow++;
    }
}

//打印字符
void printc(char c,uint32_t color){
    //如果过界
    if(PrintLine > (ScreenHeigth / 16)){
        PrintLine = 0;
    }
    //如果字符为换行符
    if(c == '\n'){
        PrintLine += 1;
        PrintRow = 0;
        return;
    }
    //如果字符为换行符
    if(c == '\t'){
        DrawString1("\t",PrintRow * 10,PrintLine * 18,color);
        return;
    }
    DrawChar(c,PrintRow * 10,PrintLine * 18,color);//绘制文字
    PrintRow += 1;//记录打印位置
}

//打印字符串
void print(char *s,uint32_t color){
    //如果过界
    if(PrintLine > (ScreenHeigth / 16)){
        PrintLine = 0;
    }
    DrawString1(s,PrintRow * 10,PrintLine * 18,color);//绘制文字
    //如果有换行符
    if(LastCharOf(s) == '\n'){
        PrintRow = 0;
    }
}

#ifndef __GNUC__
#error "This code requires GCC/Clang builtins for va_list"
#endif

typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

//将字符串复制到目标位置
void strcpy_to_buffer(char **dest_ptr, const char *src) {
    while (*src) {
        *(*dest_ptr)++ = *src++;
    }
}

//将单个字符复制到目标位置
void char_to_buffer(char **dest_ptr, char c) {
    *(*dest_ptr)++ = c;
}

//vsprintf实现，供sprintf和printf调用
static int vsprintf(char *str, const char *format, va_list args) {
    char *dest = str;
    char buffer[512];//临时缓冲区
    while (*format) {
        if (*format == '%') {
            format++;//跳过'%'
            switch (*format) {
                case 'c': {
                    //字符
                    char c = (char)va_arg(args, int);//char在可变参数中被提升为int
                    char_to_buffer(&dest, c);
                    break;
                }
                case 's': {
                    //字符串
                    char *s = va_arg(args, char*);
                    if (s) {
                        strcpy_to_buffer(&dest, s);
                    } else {
                        strcpy_to_buffer(&dest, "(null)");
                    }
                    break;
                }
                case 'd': {
                    //有符号十进制整数
                    int value = va_arg(args, int);
                    size_t len = itoa_dec(value, buffer);
                    strcpy_to_buffer(&dest, buffer);
                    break;
                }
                case 'x': {
                    //十六进制整数（小写），保留0x前缀
                    unsigned int value = va_arg(args, unsigned int);
                    size_t len = itoa_hex(value, buffer);
                    //转换为小写
                    for (size_t i = 2; i < len; i++) {
                        if (buffer[i] >= 'A' && buffer[i] <= 'F') {
                            buffer[i] = buffer[i] + ('a' - 'A');
                        }
                    }
                    strcpy_to_buffer(&dest, buffer);
                    break;
                }
                case 'X': {
                    //十六进制整数（大写），保留0x前缀
                    unsigned int value = va_arg(args, unsigned int);
                    size_t len = itoa_hex(value, buffer);//itoa_hex已生成大写带0x
                    strcpy_to_buffer(&dest, buffer);
                    break;
                }
                case 'p': {
                    //指针（64位地址）
                    void *ptr = va_arg(args, void*);
                    uintptr_t value = (uintptr_t)ptr;
                    size_t len = itoa_hex(value, buffer);//使用64位版本的转换
                    //转换为小写显示
                    for (size_t i = 2; i < len; i++) {
                        if (buffer[i] >= 'A' && buffer[i] <= 'F') {
                            buffer[i] = buffer[i] + ('a' - 'A');
                        }
                    }
                    strcpy_to_buffer(&dest, buffer);
                    break;
                }
                case '%': {
                    //转义的百分号
                    char_to_buffer(&dest, '%');
                    break;
                }
                default: {
                    //未知格式，原样输出
                    char_to_buffer(&dest, '%');
                    char_to_buffer(&dest, *format);
                    break;
                }
            }
            format++;
        } else {
            //普通字符
            char_to_buffer(&dest, *format);
            format++;
        }
    }
    *dest = '\0';//添加字符串终止符
    return (int)(dest - str);//返回写入的字符数（不包括终止符）
}

//sprintf实现
int sprintf(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsprintf(str, format, args);
    va_end(args);
    return result;
}

//printf实现
int printf(const char *format, ...) {
    char buffer[512];//静态缓冲区
    va_list args;
    va_start(args, format);
    int result = vsprintf(buffer, format, args);
    va_end(args);
    print(buffer, COLOR_WHITE);
    return result;
}

void out_error(char* text){
    printc('[',COLOR_WHITE);
    print("ERROR",COLOR_RED);
    printc(']',COLOR_WHITE);
    print(text,COLOR_WHITE);
}

void out_waring(char* text){
    printc('[',COLOR_WHITE);
    print("WARING",COLOR_YELLOW);
    printc(']',COLOR_WHITE);
    print(text,COLOR_WHITE);
}

void out_ok(char* text,uint8_t mode){
    printc('[',COLOR_WHITE);
    if(mode == 1){
        print("SUCESS",COLOR_GREEN);
    }else{
        print("OK",COLOR_GREEN);
    }
    printc(']',COLOR_WHITE);
    print(text,COLOR_WHITE);
}

void out_debug(char* text,int16_t delay){
    printc('[',COLOR_WHITE);
    print("DEBUG",COLOR_BLUE);
    printc(']',COLOR_WHITE);
    print(text,COLOR_WHITE);
    if(delay > 0){
        delay_ms(delay);
    }else if(delay == 0){
        return;
    }else{
        SYSTEM_STOP();
    }
}