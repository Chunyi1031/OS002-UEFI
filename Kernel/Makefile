#
# OS002 内核构建脚本
#
# 默认工具
#	CC:x86_64-linux-gnu-gcc
#	LD:x86_64-linux-gnu-ld
#	OBJCOPY:x86_64-linux-gnu-objcopy
#
# 2026/3/7 Liu Chunyi
#

.SILENT:

#GNU-GCC工具链
CC = x86_64-linux-gnu-gcc			#CC编译器
LD = x86_64-linux-gnu-ld			#LD链接器
OBJCOPY = x86_64-linux-gnu-objcopy	#OBJCOPY对象转换器

#目录
TMP_DIR = tmp
BUILD_DIR = build
MAIN_DIR = kernel

#文件
MAIN_FILE = $(MAIN_DIR)/main.c
TOOLS_FILE = $(MAIN_DIR)/Tools.c
DRAW_FILE = $(MAIN_DIR)/Draw.c
KEYBOARD_FILE = $(MAIN_DIR)/drivers/Keyboard.c
POWER_FILE = $(MAIN_DIR)/acpi/Power.c
PRINT_FILE = $(MAIN_DIR)/Print.c
ACPI_FILE = $(MAIN_DIR)/acpi/ACPI.c
MMD_FILE = $(MAIN_DIR)/mm/MemoryDescriptor.c
FAMM_FILE = $(MAIN_DIR)/mm/FAMemory.c
PMM_FILE = $(MAIN_DIR)/mm/PMemory.c
IDT_FILE = $(MAIN_DIR)/interrupt/IDT.c
PIC_FILE = $(MAIN_DIR)/interrupt/PIC.c
TIME_FILE = $(MAIN_DIR)/drivers/Time.c
TASK_FILE = $(MAIN_DIR)/Task.c
BITMAP_FILE = $(MAIN_DIR)/mm/Bitmap.c

MAIN_OBJ_FILE = $(TMP_DIR)/main.o
TOOLS_FILE_OBJ = $(TMP_DIR)/Tools.o
DRAW_FILE_OBJ = $(TMP_DIR)/Draw.o
KEYBOARD_FILE_OBJ = $(TMP_DIR)/Keyboard.o
POWER_FILE_OBJ = $(TMP_DIR)/Power.o
PRINT_FILE_OBJ = $(TMP_DIR)/Print.o
ACPI_FILE_OBJ = $(TMP_DIR)/ACPI.o
MMD_FILE_OBJ = $(TMP_DIR)/MemoryDescriptor.o
PMM_FILE_OBJ = $(TMP_DIR)/PMemory.o
MEMORY_OBJ = $(TMP_DIR)/Memory.o
IDT_FILE_OBJ = $(TMP_DIR)/IDT.o
PIC_FILE_OBJ = $(TMP_DIR)/PIC.o
INTERRUPT_OBJ = $(TMP_DIR)/Interrupt.o
TIME_FILE_OBJ = $(TMP_DIR)/Time.o
TASK_FILE_OBJ = $(TMP_DIR)/Task.o
BITMAP_FILE_OBJ = $(TMP_DIR)/Bitmap.o
VMM_FILE_OBJ = $(TMP_DIR)/VMemory.o

LINK_FILE = kernel.ld
ELF_FILE = $(TMP_DIR)/kernel.elf
TARGET = $(BUILD_DIR)/kernel

#编译参数
CC_OPT = -c -ffreestanding -fno-stack-protector -mno-red-zone -fno-builtin -I ./kernel -I ./kernel/include -nostdlib -m64
LD_OPT = -nostdlib -static -T $(LINK_FILE) -e _start --gc-sections --build-id=none
OBJCOPY_OPT = -O binary -R .note -R .comment -S

all: $(TARGET)
clean:
	rm -rf build/* tmp/*

#重点文件不优化
$(MAIN_OBJ_FILE):$(MAIN_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O0 $< -o $@
$(TOOLS_FILE_OBJ):$(TOOLS_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O0 $< -o $@
$(DRAW_FILE_OBJ):$(DRAW_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O0 $< -o $@
$(PRINT_FILE_OBJ):$(PRINT_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O0 $< -o $@
$(KEYBOARD_FILE_OBJ):$(KEYBOARD_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(ACPI_FILE_OBJ):$(ACPI_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(POWER_FILE_OBJ):$(POWER_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(MMD_FILE_OBJ):$(MMD_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(FAMM_FILE_OBJ):$(FAMM_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(PMM_FILE_OBJ):$(PMM_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(IDT_FILE_OBJ):$(IDT_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(PIC_FILE_OBJ):$(PIC_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(TIME_FILE_OBJ):$(TIME_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O2 $< -o $@
$(TASK_FILE_OBJ):$(TASK_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O0 $< -o $@
$(BITMAP_FILE_OBJ):$(BITMAP_FILE)
	echo "CC $@"
	$(CC) $(CC_OPT) -O0 $< -o $@

$(INTERRUPT_OBJ):$(IDT_FILE_OBJ) $(PIC_FILE_OBJ)
	echo "LD $@"
	$(LD) -r $^ -o $@
$(MEMORY_OBJ):$(MMD_FILE_OBJ) $(PMM_FILE_OBJ) $(BITMAP_FILE_OBJ)
	echo "LD $@"
	$(LD) -r $^ -o $@

$(ELF_FILE):$(MAIN_OBJ_FILE) $(TOOLS_FILE_OBJ) $(DRAW_FILE_OBJ) $(PRINT_FILE_OBJ) $(TIME_FILE_OBJ) $(MEMORY_OBJ) $(INTERRUPT_OBJ) $(TASK_FILE_OBJ) $(ACPI_FILE_OBJ) $(POWER_FILE_OBJ) $(KEYBOARD_FILE_OBJ)
	echo "LD $@"
	$(LD) $(LD_OPT) $^ -o $@

$(TARGET):$(ELF_FILE)
	echo "OBJCOPY $@"
	$(OBJCOPY) $(OBJCOPY_OPT) $< $(TMP_DIR)/kernel.bin
	cat head.bin $(TMP_DIR)/kernel.bin > $@
	rm $(TMP_DIR)/kernel.bin
	echo "Build successful"
	echo "File:$@"

.PHONY:all clean
