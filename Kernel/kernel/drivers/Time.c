#include <Time.h>

void init_rtc(){
    //周期中断
    cmos_write(CMOS_B,0b01000010);
    cmos_read(CMOS_C);
    outb(CMOS_A,(inb(CMOS_A) & 0xF) | 0b1110);//设置中断频率
}

uint8_t cmos_read(uint8_t addr){
    outb(CMOS_ADDR,CMOS_NMI | addr);
    return inb(CMOS_DATA);
}

void cmos_write(uint8_t addr, uint8_t value){
    outb(CMOS_ADDR,CMOS_NMI | addr);
    outb(CMOS_DATA,value);
}

// 检查RTC是否正在更新
int rtc_updating(void) {
    outb(CMOS_ADDR, 0x0A);
    return inb(CMOS_DATA) & 0x80;
}

// 获取RTC时间
void get_rtc_time(struct rtc_time *time) {
    while (rtc_updating());//等待RTC不忙
    time->second = cmos_read(RTC_SECONDS);
    time->minute = cmos_read(RTC_MINUTES);
    time->hour = cmos_read(RTC_HOURS);
    time->day = cmos_read(RTC_DAY);
    time->month = cmos_read(RTC_MONTH);
    time->year = cmos_read(RTC_YEAR);
    uint8_t reg_b = cmos_read(RTC_STATUS_B);//读取状态寄存器B
    //如果使用BCD格式，转换为二进制
    if (!(reg_b & 0x04)) {
        time->second = (time->second & 0x0F) + ((time->second / 16) * 10);
        time->minute = (time->minute & 0x0F) + ((time->minute / 16) * 10);
        time->hour = ((time->hour & 0x0F) + (((time->hour & 0x70) / 16) * 10)) 
                     | (time->hour & 0x80);
        time->day = (time->day & 0x0F) + ((time->day / 16) * 10);
        time->month = (time->month & 0x0F) + ((time->month / 16) * 10);
        time->year = (time->year & 0x0F) + ((time->year / 16) * 10);
    }
    //如果是12小时制，转换为24小时制
    if (!(reg_b & 0x02) && (time->hour & 0x80)) {
        time->hour = ((time->hour & 0x7F) + 12) % 24;
    }
    time->hour += 0;
    if(time->hour > 24){
        time->hour -= 24;
        time->day += 1;
    }
}