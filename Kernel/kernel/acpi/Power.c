/*
参考:
- https://github.com/Chunyi1031/OS001-UEFI/blob/main/Kernel/kernel/Power.c
*/

#include <Power.h>

void shutdown_acpi(){
    ACPI_SDT_HEADER* FADT_Header = find_FADT();//获取ACPI起始地址
    if(!FADT_Header){
        FADT_Header = find_FADT_in_RSDT();//在RSDT中查找ACPI起始地址
    }
    if(!FADT_Header){
        out_error("FADT not found!\n");
        return;
    }
    FADT_DESCRIPTOR* FADT = (FADT_DESCRIPTOR*)FADT_Header;//设置FADT指针
    //获取PM1a控制块地址
    uint32_t pm1a_cnt_addr = FADT->PM1a_CNT_BLK;
    if(pm1a_cnt_addr == 0) {
        out_error("PM1a control block not found\n");
        return;
    }
    // 在 FADT 中查找 S5 睡眠类型（通常偏移量为 0x8C）
    uint8_t* fadt_bytes = (uint8_t*)FADT;
    uint8_t slp_typa = fadt_bytes[0x8C];  // SLP_TYPa
    uint8_t slp_typb = fadt_bytes[0x8D];  // SLP_TYPb
    uint8_t slp_en = 0x1;                 // SLP_EN 位
    uint16_t pm1a_value = 0;
    //设置 SCI_EN（ACPI 系统控制中断使能）
    pm1a_value |= (1 << 9);
    //设置 S5 睡眠类型（S5 通常是 5 或 7）
    pm1a_value |= ((slp_typa & 0x5) << 10);
    //设置 SLP_EN 位以触发睡眠/关机
    pm1a_value |= (slp_en << 13);
    if (FADT->PM1b_CNT_BLK)outw(FADT->PM1b_CNT_BLK, pm1a_value);
    //写入 PM1a 控制寄存器
    outw(pm1a_cnt_addr, pm1a_value);
    //短暂延迟
    delay_ms(10);
    //如果上述方法失败，尝试直接写入 SLP_TYPa 和 SLP_EN
    pm1a_value = ((slp_typa & 0x7) << 10) | (1 << 13);
    outw(pm1a_cnt_addr, pm1a_value);
}

void shutdown_vm(){
    asm volatile("cli");//禁用中断
    outw(0x4004, 0x3400);//VirtualBox
    outw(0x4004, 0x3400);//VMware
    outw(0x48B0, 0x1234); //VMware备用端口
    outw(0x604, 0x2000);//QEMU/KVM
    //Bochs
    outw(0x8900, 0x00);
    outw(0x8900, 'S');
    outw(0x8900, 'h');
    outw(0x8900, 'u');
    outw(0x8900, 't');
    outw(0x8900, 'd');
    outw(0x8900, 'o');
    outw(0x8900, 'w');
    outw(0x8900, 'n');
    outw(0xB004, 0x2000);//旧版QEMU/Bochs
    outb(0x5100, 0x01);//Xen
}

void reboot_acpi() {
    ACPI_SDT_HEADER* FADT_Header = find_FADT();
    if (!FADT_Header) {
        FADT_Header = find_FADT_in_RSDT();
    }
    if (!FADT_Header) {
        out_error("FADT not found!\n");
        return;
    }
    FADT_DESCRIPTOR* FADT = (FADT_DESCRIPTOR*)FADT_Header;
    //检查RESET_REG是否有效
    if (FADT->RESET_REG.Address == 0) {
        out_error("ACPI reset register not available\n");
        return;
    }
    //执行重启（通常为I/O空间）
    if(FADT->RESET_REG.AddressSpaceID == 1){
        outb((uint32_t)FADT->RESET_REG.Address, (uint32_t)FADT->RESET_VALUE);
    }
    // 等待重启
    delay_ms(10);
}

// 执行关机
void SYSTEM_SHUTDOWN(void) {
    disable_interrupts();
    shutdown_acpi();
    out_error("Shutdown Failed!Mode:ACPI\n");
    delay_seconds(1);//延迟
    shutdown_vm();
    asm("pause");
    //如果以上都失败，强制重启
    out_error("Shutdown Failed!Mode:VM\n");
    out_error("Rebooting...\n");
    delay_seconds(2);
    SYSTEM_REBOOT();
}

void SYSTEM_REBOOT(){
    disable_interrupts();
    reboot_acpi();
    out_error("Reboot Failed!Mode:ACPI\n");
    delay_seconds(1);//延迟
    outb(0x64, 0xFE);//复位
    asm("pause");
    out_error("Reboot Failed!Mode:VM\n");
    out_error("Stopping...\n");
    delay_seconds(2);
    SYSTEM_STOP();//如果失败，则挂起
}