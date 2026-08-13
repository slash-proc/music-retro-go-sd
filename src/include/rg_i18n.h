#pragma once
#include <stdint.h>

#define ODROID_DIALOG_CHOICE_SEPARATOR {0x0F0F0F0E, "-", "-", -1, NULL}

/* English fallbacks for Music strings (firmware lang_t stays private). */
typedef struct {
    const char *s_Full;
    const char *s_Fill;
    const char *s_Brightness;
    const char *s_Volume;
    const char *s_Quit_to_menu;
    const char *s_favorite;
    const char *s_music;
    const char *s_repeat;
    const char *s_shuffle;
    const char *s_info;
    const char *s_lyrics;
    const char *s_empty_music;
    const char *s_no_favorite;
} lang_t;

extern lang_t *curr_lang;

int i18n_get_text_width(const char *text);
int i18n_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width,
                        const char *text, uint16_t color, uint16_t color_bg,
                        char transparent);
void odroid_overlay_clock(int x_pos, int y_pos);
