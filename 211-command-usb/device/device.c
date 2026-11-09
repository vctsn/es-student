#include "device.h"
#include "pico/version.h"
#include <stdio.h>
#include "pico/unique_id.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"
#include <stddef.h>

struct info_t device_card = {
    .version = 100,
    .name = DEVICE_NAME ,
    .revision = 1
};

void dev_info()
{
    printf("struct          address     size offset value\n");

    printf("device_card     0x%08x %5u\n", 
           (unsigned int)&device_card, (unsigned int)sizeof(device_card));

    printf("- %-13s 0x%08x %5u %6u %u\n", 
           "revision", (unsigned int)&device_card.revision, 
           (unsigned int)sizeof(device_card.revision), 
           (unsigned int)offsetof(struct info_t, revision), 
           device_card.revision);

    printf("- %-13s 0x%08x %5u %6u 0x%08x\n", 
           "version", (unsigned int)&device_card.version, 
           (unsigned int)sizeof(device_card.version), 
           (unsigned int)offsetof(struct info_t, version), 
           device_card.version);

    printf("- %-13s 0x%08x %5u %6u %s\n", 
           "name", (unsigned int)&device_card.name, 
           (unsigned int)sizeof(device_card.name), 
           (unsigned int)offsetof(struct info_t, name), 
           device_card.name);

    unsigned fields = sizeof(device_card.version) + sizeof(device_card.name) + sizeof(device_card.revision);
    printf("fields %u, sizeof %u, padding %u\n", 
           fields, (unsigned int)sizeof(device_card), (unsigned int)sizeof(device_card) - fields);
}

void device_info()
{
    char board_id[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    pico_get_unique_board_id_string(board_id, sizeof(board_id));

    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;


    uint32_t manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    int32_t part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
    int32_t revision = (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;

    printf("project: %s\n", DEVICE_PROJECT);
    printf("repo: %s\n", DEVICE_REPO);
    printf("board: %s\n", DEVICE_BOARD);
    printf("serial: %s\n", board_id);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n", manufacturer, part, revision);
    printf("pico-sdk: %s\n", PICO_SDK_VERSION_STRING);
}

