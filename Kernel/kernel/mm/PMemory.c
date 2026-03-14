/*
参考:
    https://www.bilibili.com/video/BV1LR4y1u7qC?t=633.5
*/

#include <Memory.h>

//全局变量：存储初始化后的OS内存描述符数组
OS_MEMORY_DESCRIPTOR* gOSMemoryDescriptors=NULL;
uint32_t gOSMemDescCount=0;

STATUS InitPhysicalMemoryManager(MEMORY_MAP* MemoryMap){//物理内存管理器初始化
    STATUS Status=CheckMemoryMap(MemoryMap);//检查内存映射
    if(Status!=STATUS_SUCCESS)return Status;
    uintptr_t OSMemoryAddr=(uintptr_t)AllocatePage(((MemoryMap->MapSize>>12)+1));//开辟内存
    if(!OSMemoryAddr)return 2;
    gOSMemoryDescriptors=(OS_MEMORY_DESCRIPTOR*)OSMemoryAddr;//设置地址
    uint32_t descCount=MemoryMap->MapSize/MemoryMap->DescriptorSize;//计算描述符数量
    MEMORY_DESCRIPTOR* desc=NULL;
    OS_MEMORY_DESCRIPTOR* prevOSMemory=NULL;//上一个内存描述符
    gOSMemDescCount=0;//初始化计数器
    for(uint32_t i=0;i<descCount;i++){//遍历所有描述符
        desc=AnalysisMemoryMap(MemoryMap,i);//解析描述符
        if(((desc->Type==BOOT_SERVICES_CODE)||(desc->Type==BOOT_SERVICES_DATA)||(desc->Type==CONVENTIONAL_MEMORY)) && desc->PhysicalStart){
            gOSMemoryDescriptors[gOSMemDescCount].Type=CONVENTIONAL_MEMORY;
        }else if(desc->Type==MEMORY_MAPPED_IO){
            gOSMemoryDescriptors[gOSMemDescCount].Type=MEMORY_MAPPED_IO;
        }
        gOSMemoryDescriptors[gOSMemDescCount].PhysicalStart=desc->PhysicalStart;
        gOSMemoryDescriptors[gOSMemDescCount].PageSize=desc->NumberOfPages;
        //合并相邻的相同类型内存块
        if(prevOSMemory!=NULL&&prevOSMemory->Type==gOSMemoryDescriptors[gOSMemDescCount].Type){
            if(gOSMemoryDescriptors[gOSMemDescCount].PhysicalStart==(prevOSMemory->PhysicalStart+(prevOSMemory->PageSize<<12))){
                prevOSMemory->PageSize+=gOSMemoryDescriptors[gOSMemDescCount].PageSize;
                continue;//跳过当前项
            }
        }
        prevOSMemory=&gOSMemoryDescriptors[gOSMemDescCount];//更新上一个节点
        gOSMemDescCount++;//增加计数器
    }
    //为每个块分配位图
    for (uint32_t i = 0; i < gOSMemDescCount; i++) {
        OS_MEMORY_DESCRIPTOR* desc = &gOSMemoryDescriptors[i];
        //只对可用内存块建立位图
        if (desc->Type != CONVENTIONAL_MEMORY || desc->PageSize <= 2) continue;
        uint64_t total_pages = desc->PageSize;
        uint64_t bitmap_bytes = (total_pages + 7) / 8;
        uint64_t bitmap_pages = (bitmap_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
        //从当前块的起始处分配位图页
        uintptr_t bitmap_phys = desc->PhysicalStart;
        desc->PhysicalStart += bitmap_pages * PAGE_SIZE;//调整可用起始地址
        desc->PageSize -= bitmap_pages;//减少可用页数
        //位图虚拟地址（分页开启前可以直接使用物理地址）
        uint8_t* bitmap = (uint8_t*)(uintptr_t)bitmap_phys;
        desc->bitmap = bitmap;
        //初始化位图：先将所有页标记为已分配（1）
        memset(bitmap, 0xFF, bitmap_pages * PAGE_SIZE);//整块设为1
        //然后将除位图页外的其他页标记为空闲（0）
        for (uint64_t p = bitmap_pages; p < total_pages; p++) {
            uint64_t byte = p / 8;
            uint64_t bit = p % 8;
            bitmap[byte] &= ~(1 << bit);   // 清0表示空闲
        }
    }
    return STATUS_SUCCESS;
}

// 分配多页
void* pmm_alloc_pages(size_t count) {
    if (count == 0) return NULL;

    for (uint32_t i = 0; i < gOSMemDescCount; i++) {
        OS_MEMORY_DESCRIPTOR* desc = &gOSMemoryDescriptors[i];
        if (desc->Type != CONVENTIONAL_MEMORY || desc->bitmap == NULL)
            continue;

        uint64_t total_pages = desc->PageSize;
        uint8_t* bitmap = desc->bitmap;
        uintptr_t start_phys = desc->PhysicalStart;

        uint64_t consecutive = 0;
        for (uint64_t p = 0; p < total_pages; p++) {
            uint64_t byte = p / 8;
            uint64_t bit = p % 8;

            if (!(bitmap[byte] & (1 << bit))) {
                consecutive++;
                if (consecutive == count) {
                    // 找到连续块，起始页为 p - count + 1
                    uint64_t start_page = p - count + 1;
                    // 标记这些页为已用
                    for (uint64_t j = 0; j < count; j++) {
                        uint64_t cur_page = start_page + j;
                        uint64_t cur_byte = cur_page / 8;
                        uint64_t cur_bit = cur_page % 8;
                        bitmap[cur_byte] |= (1 << cur_bit);
                    }
                    return (void*)(start_phys + start_page * PAGE_SIZE);
                }
            } else {
                consecutive = 0;
            }
        }
    }
    return NULL;
}

// 释放多页
void pmm_free_pages(void* addr, size_t count) {
    if (addr == NULL || count == 0) return;
    uintptr_t phys = (uintptr_t)addr;
    for (uint32_t i = 0; i < gOSMemDescCount; i++) {
        OS_MEMORY_DESCRIPTOR* desc = &gOSMemoryDescriptors[i];
        if (desc->Type != CONVENTIONAL_MEMORY || desc->bitmap == NULL)
            continue;

        uintptr_t start = desc->PhysicalStart;
        uint64_t total_pages = desc->PageSize;
        uintptr_t end = start + total_pages * PAGE_SIZE;

        if (phys >= start && phys < end) {
            uint64_t start_page = (phys - start) / PAGE_SIZE;
            // 检查是否越界
            if (start_page + count > total_pages) {
                // 错误：试图释放超出区域范围的页
                return;
            }
            // 清除对应位图位
            for (uint64_t j = 0; j < count; j++) {
                uint64_t cur_page = start_page + j;
                uint64_t cur_byte = cur_page / 8;
                uint64_t cur_bit = cur_page % 8;
                desc->bitmap[cur_byte] &= ~(1 << cur_bit);
            }
            return;
        }
    }
}