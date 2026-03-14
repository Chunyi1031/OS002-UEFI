/*
参考:
    https://github.com/Chunyi1031/OS001-UEFI/blob/main/Kernel/kernel/Keyboard.c
*/

#include <Keyboard.h>

const char scan_to_ascii[128] = {
    0,  0, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t', // Backspace=0x0E, Tab=0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0, 'a', 's',   // Enter=0x1C
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};
const char scan_to_ascii_1[128] = {
    0,  0, '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\b', '\t', // Backspace=0x0E, Tab=0x0F
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', '\n', 0, 'A', 'S',   // Enter=0x1C
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

int shift_pressed = 0;

int kbhit(void) {
    return (inb(0x64) & 0x01); // OUT_BUF_FULL
}
uint8_t read_scan_code(void) {
    while (!kbhit());//等待有数据可读
    return inb(0x60);//返回扫描码
}
char GetKey(void) {
    while (1) {
        uint8_t scancode = read_scan_code();
        // 处理Shift键状态
        if (scancode == LeftShift || scancode == RightShift) { // Shift按下
            shift_pressed = 1;
            continue;
        } else if (scancode == (LeftShift | 0x80) || scancode == (RightShift | 0x80)) { // Shift释放
            shift_pressed = 0;
            continue;
        }
        // 忽略其他按键的释放事件（Shift已在上方处理）
        if (scancode & 0x80) {
            continue;
        }
        // 根据Shift状态选择转换表
        char c = shift_pressed ? scan_to_ascii_1[scancode] : scan_to_ascii[scancode];
        return c != 0 ? c : (char)0;
    }
}
char GetKey_NoBlock(void) {
    while(1){
        uint8_t scancode;
        if(kbhit()){
            scancode = inb(0x60);
        }else{
            return 0;
        }
        // 处理Shift键状态
        if (scancode == LeftShift || scancode == RightShift) { // Shift按下
            shift_pressed = 1;
            continue;
        } else if (scancode == (LeftShift | 0x80) || scancode == (RightShift | 0x80)) { // Shift释放
            shift_pressed = 0;
            continue;
        }
        // 忽略其他按键的释放事件（Shift已在上方处理）
        if (scancode & 0x80) {
            continue;
        }
        // 根据Shift状态选择转换表
        char c = shift_pressed ? scan_to_ascii_1[scancode] : scan_to_ascii[scancode];
        return c != 0 ? c : (char)0;
    }
}