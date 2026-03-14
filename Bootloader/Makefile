#
# OS002 引导程序构建脚本
#
# 默认工具
#	CC:x86_64-linux-gnu-gcc
#	LD:x86_64-linux-gnu-ld
#	OBJCOPY:x86_64-linux-gnu-objcopy
#
# 2026/3/7 Liu Chunyi
#

.SILENT:

# 工具链设置
ARCH = x86_64
CC = x86_64-linux-gnu-gcc			#CC编译器
LD = x86_64-linux-gnu-ld			#LD链接器
OBJCOPY = x86_64-linux-gnu-objcopy	#OBJCOPY对象转换器

# 路径设置
BUILD_DIR = ./build
TMP_DIR = ./tmp
MAIN_DIR = ./main
GNUEFI_DIR = ./GNU-EFI
GNUEFI_INC = $(GNUEFI_DIR)/inc
MNT_DIR = ./mnt

# 文件设置
MAIN_FILE = $(MAIN_DIR)/main.c
LIB_TOOLS = Tools.c

OBJ_LIB_TOOLS= Tools.o
OBJ_FILE = $(TMP_DIR)/main.o

SO_FILE = $(TMP_DIR)/BOOT.SO
TARGET = $(BUILD_DIR)/BOOTx64.EFI

all: $(TARGET)

clean: 
	rm -rf $(BUILD_DIR)/* $(TMP_DIR)/*

#OBJ文件编译
$(OBJ_FILE): $(MAIN_FILE)
	echo "CC Tools.c"
	$(CC) -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I $(GNUEFI_INC) -I $(GNUEFI_INC)/$(ARCH) -I $(MAIN_DIR) -DEFI_FUNCTION_WRAPPER \
		$(MAIN_DIR)/$(LIB_TOOLS) -o $(TMP_DIR)/$(OBJ_LIB_TOOLS)
	echo "CC main.c"
	$(CC) -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I $(GNUEFI_INC) -I $(GNUEFI_INC)/$(ARCH) -I $(MAIN_DIR) -DEFI_FUNCTION_WRAPPER \
		$< -o $@
#链接为SO文件
$(SO_FILE): $(OBJ_FILE)
	echo "Linking"
	$(LD) -shared -Bsymbolic -nostdlib -znocombreloc -T $(GNUEFI_DIR)/elf_x86_64_efi.lds $(GNUEFI_DIR)/crt0-efi-x86_64.o \
		$< $(TMP_DIR)/$(OBJ_LIB_TOOLS) \
		-L $(GNUEFI_DIR) -lefi -lgnuefi -o $@
#输出EFI文件
$(TARGET): $(SO_FILE)
	echo "Objcopy"
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-$(ARCH) $< $@
	echo "Build successful"
	echo "File:$@"

.PHONY: all clean