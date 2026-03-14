/*
    Keyboard.h Keyboard.c
 键盘管理库
-  读取键盘的键值并转换为ASCII码

2026/3/6 Liu Chunyi

*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Tools.h>

// Control 键
#define LeftCtrl   0x1D  // 左Control键
#define RightCtrl  0xE01D // 右Control键（扩展扫描码）

// Alt 键  
#define LeftAlt    0x38  // 左Alt键
#define RightAlt   0xE038 // 右Alt键（扩展扫描码）

// Shift 键
#define LeftShift  0x2A  // 左Shift
#define RightShift 0x36  // 右Shift

// 箭头键
#define UpArrow    0x48  // 上箭头键
#define DownArrow  0x50  // 下箭头键  
#define LeftArrow  0x4B  // 左箭头键
#define RightArrow 0x4D  // 右箭头键

extern const char scan_to_ascii[128];//ASCII表
int kbhit(void);//检查键盘是否可用
uint8_t read_scan_code(void);//读取扫描码
char GetKey(void);//读取一个按键
char GetKey_NoBlock(void);//读取一个按键(非阻塞)

#endif