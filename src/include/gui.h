#pragma once
#include <stdint.h>

typedef struct {
    uint16_t bg_c;
    uint16_t main_c;
    uint16_t sel_c;
    uint16_t dis_c;
} colors_t;

/* Live launcher theme via ABI (overridden by gw_core_bridge.h macro). */
extern colors_t *curr_colors;
