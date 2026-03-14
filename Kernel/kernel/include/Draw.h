/*
    Draw.h Draw.c
 屏幕绘制库
- 定义对屏幕的绘制，显示文字，颜色

2026/3/6 Liu Chunyi

*/

#ifndef DRAW_H
#define DRAW_H

#include <Tools.h>

extern uint32_t *FrameBuffer;
extern uint16_t ScreenWidth;
extern uint16_t ScreenHeigth;

extern const char fontlist[];//字体列表
extern uint8_t *font_data;//字体数据

//Colors:
#define COLOR_RED 0xFFFF0000
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE 0xFF0000FF
#define COLOR_YELLOW 0xFFFFFF00
#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_GREY 0xFF222222
#define COLOR_CYAN 0xFF00FFEE
#define COLOR_SKYBLUE 0xFF17B7FF

uint32_t rgb(uint8_t red, uint8_t green, uint8_t blue);//rgb转16进制
void DrawPoint(uint16_t x,uint16_t y,uint32_t color);//画点
void fillRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint32_t color);//填充矩形
void DrawChar(char c,int x,int y,uint32_t color);//显示字符
void DrawString(char *s,int x,int y,uint32_t color);//显示字符串

#endif