#include <stdio.h>
#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "memory.h"
#include "command.h"
#include <stdlib.h>
#include "device.h"
#include "led.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"

#define VECTOR_TABLE 0x10000100
volatile uint32_t *gpio_in = (volatile uint32_t *)0xd0000004;

int main(void);
uint32_t data_variable = 100;
uint32_t bss_variable;

void boot_info(void)
{
    const uint32_t *vectors = (const uint32_t *)VECTOR_TABLE;
    uint32_t stack_top = vectors[0];
    uint32_t reset_handler = vectors[1];
    uint32_t *reset_handler_f = (uint32_t *)((uintptr_t)reset_handler & ~1u);

    uint32_t level = (*gpio_in >> led_pin()) & 1u;
    uint32_t sdk_level = gpio_get(led_pin());

    printf("vector table   0x%08x\n", (unsigned int)vectors);
    printf("  stack top    0x%08x\n", (unsigned int)stack_top);
    printf("  reset        0x%08x\n", (unsigned int)reset_handler);
    printf("  reset (even) 0x%08x\n", (unsigned int)reset_handler_f);
    
    printf("gpio in        0x%08x\n", (unsigned int)gpio_in);
    printf("  led bit      %u\n", (unsigned int)level);
    printf("  gpio_get     %u\n", sdk_level);
}

void fw_info(void) 
{
    printf("object          address     value\n");

    uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
    uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);
    printf("main            0x%08x    0x%04x\n", (unsigned int)main, *main_code);
    printf("fw_info         0x%08x    0x%04x\n", (unsigned int)fw_info, *fw_info_code);

    printf("commands        0x%08x\n", (unsigned int)commands);

    for (int i = 0; i < command_count; i++) 
    { 
        printf("- %-12s  0x%08x\n", commands[i].name, (unsigned int)commands[i].handler);
    }

    printf("DEVICE_PROJECT  0x%08x    %s\n", (unsigned int)DEVICE_PROJECT, DEVICE_PROJECT);
    printf("DEVICE_BOARD    0x%08x    %s\n", (unsigned int)DEVICE_BOARD, DEVICE_BOARD);

    data_variable++;
    bss_variable++;
    printf("data_variable   0x%08x    %d\n", (unsigned int)&data_variable, data_variable);
    printf("bss_variable    0x%08x    %d\n", (unsigned int)&bss_variable, bss_variable);

    volatile uint32_t stack_variable = 1946;
    printf("stack_variable  0x%08x    %d\n", (unsigned int)&stack_variable, stack_variable);

    uint32_t *heap_variable = malloc(sizeof(uint32_t));
    if (heap_variable != NULL)
    {
           *heap_variable = 1951;
           printf("heap_variable   0x%08x    %d\n", (unsigned int)heap_variable, *heap_variable);
    }
    free(heap_variable);
}

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
    row("text",     (uintptr_t)&__boot2_end__,    (uintptr_t)&__etext);
    row("data flash", (uintptr_t)&__etext, (uintptr_t)(&__etext + data_size));
    row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
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