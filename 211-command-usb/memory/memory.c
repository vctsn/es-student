#include <stdio.h>
#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "memory.h"

#define SRAM_SIZE  (264 * 1024)   // 264 КБ из datasheet
#define ROM_SIZE   (16 * 1024)    // 16 КБ из datasheet

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;
extern char __data_start__;
extern char __data_end__;

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void mem_info(void)
{
    printf("%-10s %-10s %-10s %8s\n",
           "area", "start", "end", "size");
           uintptr_t data_size = (uintptr_t)&__data_end__ - (uintptr_t)&__data_start__;
    row("flash", XIP_BASE, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("sram", SRAM_BASE, SRAM_BASE + SRAM_SIZE);
    row("rom", ROM_BASE, ROM_BASE + ROM_SIZE);
    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free",  (uintptr_t)&__flash_binary_end, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    row("data flash", (uintptr_t)&__etext, (uintptr_t)(&__etext + data_size));
    row("data-ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
    row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    uintptr_t img_start = (uintptr_t)&__flash_binary_start; //начало образа
    uintptr_t img_end   = (uintptr_t)&__flash_binary_end; //конец образа

    uintptr_t boot2_size = (uintptr_t)&__boot2_end__ - (uintptr_t)&__boot2_start__;
    uintptr_t text_size  = (uintptr_t)&__etext - (uintptr_t)&__boot2_end__;
    uintptr_t img_size   = img_end - img_start; //размер образа
    uintptr_t flash_end = XIP_BASE + PICO_FLASH_SIZE_BYTES;
    uintptr_t free_size  = flash_end - img_end;
    uintptr_t heap_size  = (uintptr_t)&__HeapLimit - (uintptr_t)&__bss_end__;
    uintptr_t stack_size = (uintptr_t)&__StackTop - (uintptr_t)&__StackBottom;
    uintptr_t bss_size = (uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__;

    printf("total\n");
    printf("  flash image  %8u = boot2 %u + text %u + data %u\n",
           (unsigned)img_size,
           (unsigned)boot2_size,
           (unsigned)text_size,
           (unsigned)data_size);
    printf("  flash free   %8u of %u\n",
           (unsigned)(PICO_FLASH_SIZE_BYTES - img_size),
           (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("  ram used  %8u = data %u + bss %u\n",
           (unsigned)(data_size + bss_size),
           (unsigned)data_size,
           (unsigned)bss_size);
    printf("  ram free  %8u for heap and %u for stack\n",
           (unsigned)(heap_size),
           (unsigned)(stack_size));
}