/*
    Power.h Power.c
 电源管理库
- 重启和关机

2026/3/6 Liu Chunyi

*/

#ifndef POWER_H
#define POWER_H

#include <Tools.h>
#include <Draw.h>
#include <ACPI.h>
#include <Print.h>

void shutdown_acpi();//关机-ACPI
void shutdown_vm();//关机-虚拟机
void SYSTEM_SHUTDOWN();//关机
void SYSTEM_REBOOT();//重启

#endif