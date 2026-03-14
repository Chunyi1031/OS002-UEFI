/*
参考:
- https://github.com/Chunyi1031/OS001-UEFI/blob/main/Kernel/kernel/Power.c
*/

#include <ACPI.h>

ACPI_SDT_HEADER *find_FADT(){
    XSDT_DESCRIPTOR* XSDT = (XSDT_DESCRIPTOR*)RSDP->XsdtAddress;
    if(memcmp(XSDT->Signature,"XSDT",4) == 0){
        size_t entry_count = (XSDT->Length - 28) / 8;//计算指针数
        //遍历每个指针，查找“FACP”
        for (size_t i = 0; i < entry_count; i++) {
            uint64_t table_phys_addr = XSDT->Entry[i] >> 32;
            ACPI_SDT_HEADER* table_header = (ACPI_SDT_HEADER*)table_phys_addr;
            if (!table_header) continue; //映射失败，跳过
            #ifdef debug
            printf("%x:%s\n",table_phys_addr,table_header->Signature);
            #endif
            // 检查签名是否为 “FACP”
            if (memcmp(table_header, "FACP", 4) == 0) {
                return table_header; //返回FADT表头指针
            }
        }
    }
    return NULL;

}

//在RSDT中查找FADT表
ACPI_SDT_HEADER *find_FADT_in_RSDT(){
    RSDT_DESCRIPTOR* RSDT = (RSDT_DESCRIPTOR*)(uint64_t)RSDP->RsdtAddress;
    if(memcmp(RSDT->Signature,"RSDT",4) == 0){
        size_t entry_count = (RSDT->Length - 32) / 4;//计算指针数
        //遍历每个指针，查找“FACP”
        for (size_t i = 0; i < entry_count; i++) {
            uint32_t table_phys_addr = RSDT->Entry[i];
            ACPI_SDT_HEADER* table_header = (ACPI_SDT_HEADER*)(uint64_t)table_phys_addr;
            if (!table_header) continue; //映射失败，跳过
            #ifdef debug
            printf("%x:%s\n",table_phys_addr,table_header->Signature);
            #endif
            // 检查签名是否为 “FACP”
            if (memcmp(table_header, "FACP", 4) == 0) {
                return table_header; //返回FADT表头指针
            }
        }
    }
    return NULL;

}

ACPI_SDT_HEADER *find_MADT(){
    XSDT_DESCRIPTOR* XSDT = (XSDT_DESCRIPTOR*)RSDP->XsdtAddress;
    if(memcmp(XSDT->Signature,"XSDT",4) == 0){
        size_t entry_count = (XSDT->Length - 28) / 8;//计算指针数
        for (size_t i = 0; i < entry_count; i++) {
            uint64_t table_phys_addr = XSDT->Entry[i] >> 32;
            ACPI_SDT_HEADER* table_header = (ACPI_SDT_HEADER*)table_phys_addr;
            if (!table_header) continue; //映射失败，跳过
            #ifdef debug
            printf("%x:%s\n",table_phys_addr,table_header->Signature);
            #endif
            if (memcmp(table_header, "APIC", 4) == 0) {
                return table_header;
            }
        }
    }
    return NULL;

}