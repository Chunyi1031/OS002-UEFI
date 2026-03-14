# OS002 源代码

## 源代码编译环境
|项目      |要求                     |
|---------|-------------------------|
|操作系统  |Linux（建议Ubuntu/Debian） |
|编译器    |GNU-GCC(x86_64-linux-gnu-gcc)|
|Make     |GNU-Make                 |
|虚拟机    |qemu-system-x86_64       |

## 运行要求
|项目      |要求                     |
|---------|-------------------------|
|架构      |x86_64                   |
|CPU      |单核                      |
|内存      |256MB                    |
|硬盘      |128MB                    |

## 编译
- 编译`make`
- 制作虚拟硬盘`make disk`
- 复制文件到虚拟硬盘`make system`
- 运行(自动更新虚拟硬盘)`make run`
- 清理(不清理虚拟硬盘)`make clean`
- 创建系统目录 `make esp`
- 测试 `make test`
- 创建未创建的目录`make fix`

## 实体机运行
- 1.将一个小于32GB的U盘初始化为GPT分区
- 2.格式化为FAT32
- 3.输入`make esp`生成`ESP`目录
- 4.将`ESP`目录中的`SYS`和`EFI`目录复制到U盘根目录
### 注:不建议实体机运行，大概率会出现异常

# OS002 内核命令列表

## 基础命令
- `hello` - 显示 "Hello World!" 消息
- `clear` - 清屏
- `reboot` - 重启系统
- `shutdown` - 关闭系统

## 系统信息命令
- `status` - 显示系统状态信息（CPU频率、屏幕分辨率等）
- `rsdp` - 显示 RSDP 地址和 ACPI 版本
- `xsdt` - 显示 XSDT 地址
- `findFADT` - 查找并显示 FADT 地址
- `time` - 显示当前时间
- `ticks` - 显示时钟中断计数
- `IntStatus` - 显示中断状态
- `MMD` - 显示第6个内存描述符信息
- `AvaMD` - 显示可用内存信息
- `MaxMD` - 显示最大可用内存块信息

## 内存管理命令
- `AcMM` - 分配内存空间
- `FreeMM` - 释放内存空间

## 中断与设备命令
- `IntEnable` - 启用中断
- `IntDisable` - 禁用中断
- `InitINT` - 初始化中断
- `KeyBoard` - 键盘中断测试
- `beep` - 蜂鸣器测试

## 异常处理命令
- `DE` - 除零异常测试

## 多任务处理命令
- `createThread` - 创建测试线程
- `rt` - 显示正在运行的线程
- `starttest` - 开始测试线程
- `killtest` - 终止测试线程
- `sleeptest` - 线程睡眠测试
- `testmutex` - 互斥锁测试
- `sempost` - 信号量发布测试
- `semwait` - 信号量等待测试
- `seminit` - 信号量初始化
- `semtest` - 信号量测试
- `bitmaptest` - 位图测试

## 其他命令
- `clock` - 运行时钟程序（按 E 键退出）

### 2026/3/14 Liu Chunyi
### E-Mail: liuchunyi1031@163.com
