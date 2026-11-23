#include "device/device.h"
#include "pico/unique_id.h"
#include "pico/version.h"
#include "hardware/regs/sysinfo.h"
#include "hardware/structs/sysinfo.h"
#include <stddef.h>

#ifndef PICO_BOARD
#define PICO_BOARD "unknown"
#endif

void device_get_info(device_info_t *info)
{
    if (info == NULL)
    {
        return;
    }

    info->board = PICO_BOARD;
    pico_get_unique_board_id_string(info->serial, sizeof(info->serial));

    uint32_t chip_id = sysinfo_hw->chip_id;
    info->chip_manufacturer = (uint16_t)((chip_id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB);
    info->chip_part = (uint16_t)((chip_id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB);
    info->chip_revision = (uint8_t)((chip_id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB);

    info->sdk_version = PICO_SDK_VERSION_STRING;
}
