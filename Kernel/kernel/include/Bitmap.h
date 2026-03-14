/*
位图结构库
*/

#ifndef BITMAP_H
#define BITMAP_H 

#include <Tools.h>

typedef struct bitmap_t {
    uint32_t bit_size;
    uint8_t *bits;
} bitmap_t;

//计算位图大小对应的字节数
uint32_t BitmapBitSize(uint32_t bit_size);

//获取一个比特位
STATUS BitmapGetBit(bitmap_t *bitmap,uint32_t index);

void BitmapInit(bitmap_t *bitmap,uint8_t *bits,uint32_t bit_size,STATUS value);//初始化位图
int BitmapAllocBits(bitmap_t *bitmap,STATUS value,uint32_t size);//分配连续size个值为value的位
void BitmapSetBits(bitmap_t *bitmap,uint32_t index,uint32_t size,uint8_t value);//设置比特位
STATUS BitmapIsSet(bitmap_t *bitmap,uint32_t index);//设置单个比特位为1

#endif