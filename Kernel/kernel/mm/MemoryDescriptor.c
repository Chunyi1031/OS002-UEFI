#include <Memory.h>

//检查内存映射
STATUS CheckMemoryMap(MEMORY_MAP* MemoryMap){
    STATUS status = 0;
    if(!MemoryMap->Buffer)status ++;
    if(MemoryMap->MapSize == 0)status ++;
    if(MemoryMap->DescriptorSize == 0)status ++;
    if(MemoryMap->DescriptorVersion == 0)status ++;
    if(!MemoryMap->StackAddr)status ++;
    return status;
}

STATUS IsMemoryAvailable(MEMORY_DESCRIPTOR *Desc) {
    if(Desc->PhysicalStart == (uintptr_t)MemoryMap->StackAddr){
        return 4;
    }
    switch (Desc->Type) {
        //完全可用的内存类型
        case CONVENTIONAL_MEMORY:           // 常规可用内存
        case BOOT_SERVICES_CODE:            // Boot Service代码（ExitBS后可用）
        case BOOT_SERVICES_DATA:             // Boot Service数据（ExitBS后可用）
        case LOADER_CODE:                    // 加载器代码
        case LOADER_DATA:                     // 加载器数据
            return 1;
        //有条件可用/需谨慎使用
        case ACPI_RECLAIM_MEMORY:            // ACPI可回收内存
            // 在加载ACPI表后可以回收使用
            return 2;  // 但需要知道何时回收
            
        case PERSISTENT_MEMORY:               // 持久内存
            return 3;  // 特殊用途，不是常规可用内存
        //完全不可用的内存类型
        case RESERVED_MEMORY_TYPE:            // 保留内存
        case RUNTIME_SERVICES_CODE:           // 运行时服务代码
        case RUNTIME_SERVICES_DATA:           // 运行时服务数据
        case UNUSABLE_MEMORY:                 // 不可用内存
        case ACPI_MEMORY_NVS:                  // ACPI NVS内存
        case MEMORY_MAPPED_IO:                 // MMIO
        case MEMORY_MAPPED_IO_PORT_SPACE:     // I/O端口空间
        case PAL_CODE:                         // PAL代码
            return 0;
        default:
            // 未知类型，保守起见认为不可用
            return 0;
    }
}

//解析并打印描述符信息
MEMORY_DESCRIPTOR* AnalysisMemoryMap(MEMORY_MAP* MemoryMap,const uint32_t num){
    //检查映射
    STATUS _status = CheckMemoryMap(MemoryMap);
    if(_status != 0){
        out_error("Parameters invalid\n");
        return NULL;
    }
    MEMORY_DESCRIPTOR* DescriptorAddr = (MEMORY_DESCRIPTOR*)((uint8_t*)MemoryMap->Buffer + MemoryMap->DescriptorSize * num);//描述符的位置
    uint32_t maxDescriptors = MemoryMap->MapSize / MemoryMap->DescriptorSize;//计算描述符数量
    if (num >= maxDescriptors) {
        out_error("Index out of bounds\n");
        return NULL;
    }
    return DescriptorAddr;
}

//处理属性
void PrintMemoryAttributes(uint64_t Attribute) {
    printf("Attribute:    \t%X (", Attribute);
    int first = 1;
    int hasValidAttribute = 0;
    if (Attribute & MEMORY_UC) {
        printf("%sUncached", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }
    if (Attribute & MEMORY_WC) {
        printf("%sWrite Combining", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }
    if (Attribute & MEMORY_WT) {
        printf("%sWrite Through", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }
    if (Attribute & MEMORY_WB) {
        printf("%sWrite Back", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }
    if (Attribute & MEMORY_UCE) {
        printf("%sUncached Export", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }
    if (Attribute & MEMORY_WP) {
        printf("%sWrite Protect", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }
    if (Attribute & MEMORY_RP) {
        printf("%sRead Protect", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }
    if (Attribute & MEMORY_XP) {
        printf("%sExecute Protect", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }
    if (Attribute & MEMORY_RO) {
        printf("%sRead Only", first ? "" : " | ");
        first = 0;
        hasValidAttribute = 1;
    }

    // 如果没有任何有效属性，输出提示
    if (!hasValidAttribute) {
        printf("None/Unknown");
    }

    printf(")\n");
}
void PrintMemoryDescriptor(MEMORY_DESCRIPTOR* DescriptorAddr){
    int sizekb = DescriptorAddr->NumberOfPages * 4096 / 1024;
    printf("Memory descriptor at %X\n",(uint64_t)DescriptorAddr);//打印地址
    printf("Type:         \t%d\n",DescriptorAddr->Type);//Type
    printf("PhysicalStart:\t%x\n",DescriptorAddr->PhysicalStart);//物理地址
    printf("VirtualStart: \t%x\n",DescriptorAddr->VirtualStart);//虚拟地址
    printf("Pages:        \t%d\n",DescriptorAddr->NumberOfPages);//页数
    if(sizekb < 1024){
        printf("Size:         \t%dKB\n",sizekb);
    }else{
        printf("Size:         \t%dMB\n",sizekb / 1024);
    }
    printf("Available:    \t%s\n",((IsMemoryAvailable == 0) ? "False" : "True"));
    PrintMemoryAttributes(DescriptorAddr->Attribute);
}

//打印可用指定数量的内存的编号
void PrintAvailableMemoryDescriptor(MEMORY_MAP* MemoryMap,const uint32_t num){
    uint32_t maxDescriptors = MemoryMap->MapSize / MemoryMap->DescriptorSize;//计算描述符数量
    if (num >= maxDescriptors) {
        out_error("Index out of bounds\n");
        return;
    }
    int TotalNum = 0;
    MEMORY_DESCRIPTOR* DescriptorAddr = NULL;
    //遍历所有描述符
    for(int a = 0;a < maxDescriptors;a ++){
        DescriptorAddr = AnalysisMemoryMap(MemoryMap,a);
        //检查是否可用
        if(IsMemoryAvailable(DescriptorAddr) != 0){
            printf("%d ",a);
            TotalNum ++;
        }
        if(TotalNum >= num)break;//数量足够则跳出循环
    }
    printf("\nTotal:%d\n",TotalNum);
}
MEMORY_DESCRIPTOR* FindLargestAvailableBlock(MEMORY_MAP* MemoryMap) {
    // 检查 MemoryMap 是否已初始化
    if (MemoryMap == NULL) {
        out_error("MemoryMap is not initialized!\n");
        return NULL;
    }
    MEMORY_DESCRIPTOR* largest = NULL;
    uint64_t largestSize = 0;
    uint32_t maxDescriptors = MemoryMap->MapSize / MemoryMap->DescriptorSize; // 计算最大描述符数量
    // 遍历每个描述符
    for (uint32_t i = 0; i < maxDescriptors; i++) {
        MEMORY_DESCRIPTOR* Desc = (MEMORY_DESCRIPTOR*)((uint8_t*)MemoryMap->Buffer + i * MemoryMap->DescriptorSize); // 计算描述符所在内存地址
        // 只考虑完全可用的常规内存
        if (Desc->Type == CONVENTIONAL_MEMORY) {
            uint64_t size = Desc->NumberOfPages * 4096;
            if (size > largestSize) {
                largestSize = size;
                largest = Desc;
            }
        }
    }
    return largest;
}
//获取总内存大小
uint64_t GetMemoryTotalSize(MEMORY_MAP* MemoryMap) {
    // 检查 MemoryMap 是否已初始化
    if (MemoryMap == NULL) {
        out_error("MemoryMap is not initialized!\n");
        return 0;
    }
    uint32_t maxDescriptors = MemoryMap->MapSize / MemoryMap->DescriptorSize; // 计算最大描述符数量
    MEMORY_DESCRIPTOR* DescriptorAddr = NULL;
    uint64_t TotalSize = 0;
    // 遍历所有描述符
    for (int a = 0; a < maxDescriptors; a++) {
        DescriptorAddr = AnalysisMemoryMap(MemoryMap, a);
        if (DescriptorAddr->Type != MEMORY_MAPPED_IO && DescriptorAddr->Type != MEMORY_MAPPED_IO_PORT_SPACE) {
            TotalSize += DescriptorAddr->NumberOfPages * (uint64_t)4096;
        }
    }
    return TotalSize;
}
void* AllocatePage(uint64_t size){//页级分配内存
    if(MemoryMap==NULL)return NULL;//检查MemoryMap是否已初始化
    MEMORY_DESCRIPTOR* block=FindLargestAvailableBlock(MemoryMap);//查找最大的内存块的描述符
    if(block&&block->NumberOfPages>=size){//判断内存块是否足够大
        block->Type=RESERVED_MEMORY_TYPE;//标记为已分配
        return(void*)block->PhysicalStart;//返回地址
    }
    return NULL;//分配失败
}

void FreePage(void* addr){//页级回收内存
    if(!addr||MemoryMap==NULL){//检查参数有效性
        out_error("Invalid parameters for FreeMemory!\n");
        return;
    }
    uint32_t maxDescriptors=MemoryMap->MapSize/MemoryMap->DescriptorSize;//计算内存描述符数量
    for(uint32_t i=0;i<maxDescriptors;i++){//遍历所有描述符
        MEMORY_DESCRIPTOR* desc=(MEMORY_DESCRIPTOR*)((uint8_t*)MemoryMap->Buffer+i*MemoryMap->DescriptorSize);//计算描述符所在内存地址
        if(desc->PhysicalStart==(uintptr_t)addr){//如果描述的物理地址等于要回收的地址
            desc->Type=CONVENTIONAL_MEMORY;//恢复为可用
            break;//跳出循环
        }
    }
}