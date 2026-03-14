/*
参考:
    https://github.com/Chunyi1031/OS001-UEFI/blob/main/Kernel/kernel/Draw.c
*/

#include <Draw.h>

//字体列表
const char fontlist[94] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9',
    '`','~','.',',','/','\\',';','\'',':','"','<','>','(',')','[',']','{','}','#','$','%','&','*',' ',
    '!','|','@','-','+','=','_','?'

};

//rgb转16进制
uint32_t rgb(uint8_t red, uint8_t green, uint8_t blue){
    return ((uint32_t)255 << 24) | ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
}

//画点
void DrawPoint(uint16_t x,uint16_t y,uint32_t color){
    if((x >= ScreenWidth) || (y >= ScreenHeigth) || (x < 0) || (y < 0)) return;//检查是否合理
    FrameBuffer[y * ScreenWidth + x] = color;//写入颜色
}
//填充矩形
void fillRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint32_t color){
    for(int _y = y; _y < (h + y); _y ++){
        for(int _x = x; _x < (w + x); _x ++){
            DrawPoint(_x,_y,color);
        }
    }
}

//字符（串）处理
//显示字符
void DrawChar(char c,int x,int y,uint32_t color){
    int char_index = 84;
    for(int i = 0;i < 94;i ++){
        if(fontlist[i] == c){
            char_index = i;
            break;
        }
    }
    int data_offset = char_index * 32;
    for (int row = 0; row < 16; row++) {
        uint16_t row_data = (font_data[data_offset + row*2] << 8) | font_data[data_offset + row*2 + 1];
        for (int col = 0; col < 10; col++) {
            if ((row_data & (0x8000 >> col)) != 0) {
                DrawPoint(x + col, y + row, color);
            }
        }
    }
}
//显示字符串
void DrawString(char *s,int x, int y, uint32_t color) {
    int startX = x;//记录起始X坐标
    int currentX = x;
    int currentY = y;
    int charWidth = 10;//宽
    int charHeight = 16;//高
    int lineSpacing = 2;//行间距
    size_t size = strlen(s);
    for (int i = 0; i < size; i++) {
        char c = s[i];
        // 处理换行符
        if (c == '\n') {
            currentX = startX;//X坐标回到起始位置
            currentY += charHeight + lineSpacing;//Y坐标下移一行
            continue;
        }
        // 检查是否需要自动换行（如果字符串太长超出屏幕）
        int screenWidth = ScreenWidth;//假设的屏幕宽度
        if (currentX + charWidth > screenWidth) {
            currentX = startX;//X坐标回到起始位置
            currentY += charHeight + lineSpacing;//Y坐标下移一行
        }
        DrawChar(c, currentX, currentY, color);//绘制字符
        currentX += charWidth;//更新X坐标，准备绘制下一个字符
    }
}