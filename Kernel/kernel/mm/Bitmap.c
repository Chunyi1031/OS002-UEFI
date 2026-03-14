/*
参考:
    https://www.bilibili.com/video/BV1LukHYvEhV?t=2.0
*/

#include <Bitmap.h>

//计算位图大小对应的字节数
uint32_t BitmapBitSize(uint32_t bit_size){
    return (bit_size + 8 - 1) / 8;
}

//获取一个比特位
STATUS BitmapGetBit(bitmap_t *bitmap,uint32_t index){
    STATUS result = bitmap->bits[index / 8] >> (index % 8) & 0x1;
    return result;
}

//初始化位图
void BitmapInit(bitmap_t *bitmap,uint8_t *bits,uint32_t bit_size,STATUS value){
    bitmap->bit_size = bit_size;
    bitmap->bits = bits;
    int bytes = BitmapBitSize(bit_size);
    memset(bitmap->bits,value,bytes);
}

//分配连续size个值为value的位
int BitmapAllocBits(bitmap_t *bitmap,STATUS value,uint32_t size){
    int sidx = 0;
    int ridx = -1;
    while(sidx < bitmap->bit_size){
        if(BitmapGetBit(bitmap,sidx) != value){
            sidx ++;
            continue;
        }
        ridx = sidx;
        int i;
        for(i = 1;i < size && sidx < bitmap->bit_size;i ++){
            if(BitmapGetBit(bitmap,sidx++) != value){
                ridx = -1;
                break;
            }
        }
        if(i >= size){
            BitmapSetBits(bitmap,ridx,size,1);
            return ridx;
        }
    }
    return ridx;
}

//设置比特位
void BitmapSetBits(bitmap_t *bitmap,uint32_t index,uint32_t size,uint8_t value){
    for(int i = 0;i < size && index < bitmap->bit_size;i ++){
        if(value){
            bitmap->bits[index / 8] |= (1 << (index % 8));
        }else{
            bitmap->bits[index / 8] &= ~(1 << (index % 8));
        }
        index ++;
    }
}

//设置单个比特位为1
STATUS BitmapIsSet(bitmap_t *bitmap,uint32_t index){
    return BitmapGetBit(bitmap,index);
}