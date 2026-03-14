/*
    Print.h Print.c
 屏幕打印库
- 有色输出字符（串）
- 格式化字符串
- 输出信息
- 格式化输出

2026/3/6 Liu Chunyi

*/

#ifndef PRINT_H
#define PRINT_H

#include <Tools.h>
#include <Draw.h>

#define tabSize 4 //制表符大小

extern uint16_t PrintLine;//行
extern uint16_t PrintRow;//列

void print(char *s,uint32_t color);//打印字符串
void printc(char c,uint32_t color);//打印字符
int sprintf(char *str, const char *format, ...);//格式化字符串
int printf(const char *format, ...);//格式化输出
void out_error(char* text);//错误
void out_waring(char* text);//警告
void out_ok(char* text,uint8_t mode);//正常信息
void out_debug(char* text,int16_t delay);//调试信息


#endif