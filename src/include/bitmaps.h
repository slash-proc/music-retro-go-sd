#pragma once
#include <stdint.h>
#include "odroid_input.h"

/* Logo indices must match firmware Core/Inc/retro-go/bitmaps.h. */
enum {
    RG_LOGO_EMPTY = -1,
    RG_LOGO_RGO = 0,
    RG_LOGO_RGW,
};

void odroid_overlay_draw_logo(uint16_t x_pos, uint16_t y_pos, int16_t logo_idx, uint16_t color);
void odroid_overlay_draw_battery(odroid_battery_state_t battery, int x, int y);
void odroid_overlay_draw_fill_rect(int x, int y, int width, int height, uint16_t color);
