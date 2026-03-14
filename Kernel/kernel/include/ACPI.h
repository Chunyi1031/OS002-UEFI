/*
    ACPI.h ACPI.c
 ACPI库
-  对ACPI的操作，如查找FADT表

2026/3/6 Liu Chunyi

*/

#ifndef ACPI_H
#define ACPI_H

#include <Tools.h>
#include <Print.h>

//RSDP
typedef struct {
    char Signature[8];      //"RSD PTR "
    uint8_t Checksum;       //前20字节的校验和（所有字节相加的低8位必须为0）
    char OEMID[6];          //制造商ID
    uint8_t Revision;       //版本
    uint32_t RsdtAddress;   //RSDT 物理地址（32位）

    uint32_t Length;         // RSDP 结构总长度（通常为36或52）
    uint64_t XsdtAddress;    // XSDT 物理地址（64位，优先使用这个）
    uint8_t ExtendedChecksum;// 整个RSDP表的校验和（所有字节相加的低8位必须为0）
    uint8_t Reserved[3];     // 保留，必须为0
} RSDP_DESCRIPTOR;

//ACPI标准表头
typedef struct {
    char Signature[4];    //表标识
    uint32_t Length;      //整个表的长度
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} ACPI_SDT_HEADER;

// XSDT 表的具体结构
typedef struct {
    char Signature[4];    //表标识
    uint32_t Length;      //整个表的长度
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint64_t Entry[];        //指向其他ACPI表的指针数组
} XSDT_DESCRIPTOR;

// RSDT 表的具体结构
typedef struct {
    char Signature[4];    //表标识
    uint32_t Length;      //整个表的长度
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[4];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
    uint64_t Entry[];        //指向其他ACPI表的指针数组
} RSDT_DESCRIPTOR;

// 通用地址结构（用于描述寄存器地址）
typedef struct {
    uint8_t AddressSpaceID;  // 地址空间 ID（如 SystemIO = 1, Memory = 0）
    uint8_t BitWidth;        // 位宽（如 8、16、32）
    uint8_t BitOffset;       // 位偏移
    uint8_t AccessSize;      // 访问大小（如 Byte = 1, Word = 2）
    uint64_t Address;        // 寄存器地址
} GENERIC_ADDRESS_STRUCTURE;

// FADT 表结构（部分字段）
typedef struct {
    ACPI_SDT_HEADER Header;           // 表头（Signature: "FACP"）
    uint32_t FirmwareCtrl;            // 固件控制寄存器地址
    uint32_t Dsdt;                    // DSDT 地址
    uint8_t  Reserved1;               // 保留
    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;           // SCI 中断号
    uint32_t SMI_CommandPort;         // SMI 命令端口
    uint8_t  AcpiEnable;              // ACPI 启用命令
    uint8_t  AcpiDisable;             // ACPI 禁用命令
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_CNT;
    uint32_t PM1a_EVT_BLK;           // PM1a 事件块
    uint32_t PM1b_EVT_BLK;           // PM1b 事件块
    uint32_t PM1a_CNT_BLK;           // PM1a 控制块（重要！关机用）
    uint32_t PM1b_CNT_BLK;           // PM1b 控制块
    uint32_t PM2_CNT_BLK;            // PM2 控制块
    uint32_t PM_TMR_BLK;             // PM 定时器块
    uint32_t GPE0_BLK;               // GPE0 块
    uint32_t GPE1_BLK;               // GPE1 块
    uint8_t  PM1_EVT_LEN;
    uint8_t  PM1_CNT_LEN;
    uint8_t  PM2_CNT_LEN;
    uint8_t  PM_TMR_LEN;
    uint8_t  GPE0_BLK_LEN;
    uint8_t  GPE1_BLK_LEN;
    uint8_t  GPE1_BASE;
    uint8_t  CST_CNT;
    uint16_t P_LVL2_LAT;
    uint16_t P_LVL3_LAT;
    uint16_t FLUSH_SIZE;
    uint16_t FLUSH_STRIDE;
    uint8_t  DUTY_OFFSET;
    uint8_t  DUTY_WIDTH;
    uint8_t  DAY_ALRM;
    uint8_t  MON_ALRM;
    uint8_t  CENTURY;
    uint16_t IAPC_BOOT_ARCH;         // 引导架构标志
    uint8_t  Reserved2;
    uint32_t Flags;                  // 标志位
    // 重启相关字段（ACPI 2.0+）
    GENERIC_ADDRESS_STRUCTURE RESET_REG; // 重启寄存器描述符
    uint8_t RESET_VALUE;                 // 重启值

    // 其他字段（可根据需要继续补充）
    // ...
} FADT_DESCRIPTOR;

// 64位地址扩展（如果使用 XSDT）
typedef struct {
    ACPI_SDT_HEADER Header;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;
    // ... 省略中间字段
    uint32_t PM1a_CNT_BLK;           // 32位地址
    // ... 省略其他字段
    uint64_t X_PM1a_CNT_BLK;         // 64位地址（如果使用 XSDT）
    uint64_t X_PM1b_CNT_BLK;
    // ... 其他字段
    uint8_t  SLP_TYPa;               // S5 睡眠类型
    uint8_t  SLP_TYPb;
    uint8_t  SLP_EN;                 // 睡眠使能
} FADT_DESCRIPTOR_X;

extern RSDP_DESCRIPTOR *RSDP;

ACPI_SDT_HEADER *find_FADT();//寻找FADT
ACPI_SDT_HEADER *find_FADT_in_RSDT();//在RSDT中寻找FADT
ACPI_SDT_HEADER *find_MADT();//寻找MADT

#endif