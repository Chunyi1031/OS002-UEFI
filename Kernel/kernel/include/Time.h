/*
    Time.h Time.c
 时钟库

2026/3/6 Liu Chunyi

*/

#ifndef TIME_H
#define TIME_H

#include <Tools.h>

// CMOS端口定义
#define CMOS_ADDR       0x70
#define CMOS_DATA       0x71
#define CMOS_NMI        0x80

#define CMOS_A          0x0A
#define CMOS_B          0x0B
#define CMOS_C          0x0C
#define CMOS_D          0x0D

#define CMOS_SECONDS    0x01
#define CMOS_MINUTES    0x03
#define CMOS_HOURS      0x05

void init_rtc();//初始化RTC
uint8_t cmos_read(uint8_t addr);//读取RTC
void cmos_write(uint8_t addr, uint8_t value);//写入RTC

// RTC寄存器定义
#define RTC_SECONDS  0x00
#define RTC_MINUTES  0x02
#define RTC_HOURS    0x04
#define RTC_DAY      0x07
#define RTC_MONTH    0x08
#define RTC_YEAR     0x09
#define RTC_STATUS_A 0x0A
#define RTC_STATUS_B 0x0B

// 时间结构
struct rtc_time {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};

void get_rtc_time(struct rtc_time *time);

#endif