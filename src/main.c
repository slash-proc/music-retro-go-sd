/*
 * Music homebrew entry — GWHB packs call app_main via CORE_ENTRY.
 * The player implementation lives in src/music/main_music.c.
 */
#include <stdint.h>
#include "main_music.h"

void app_main(uint8_t load_state, uint8_t start_paused, int8_t save_slot)
{
    app_main_music(load_state, start_paused, save_slot);
}
