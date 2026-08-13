/*
 * Local helpers for the Music GWHB: FatFs scandir, English lang stubs,
 * and fill_rect (not on ABI — pixel loop into the active LCD buffer).
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gw_firmware_abi.h"
#include "gw_lcd.h"
#include "rg_storage.h"
#include "rg_i18n.h"
#include "ff.h"

#include "gw_core_bridge.h"

/* --- English lang (TR() falls back when curr_lang fields are set) -------- */
static lang_t g_lang_en = {
    .s_Full = "|",
    .s_Fill = ".",
    .s_Brightness = "Brightness",
    .s_Volume = "Volume",
    .s_Quit_to_menu = "Quit to menu",
    .s_favorite = "Favorites",
    .s_music = "Music",
    .s_repeat = "Repeat",
    .s_shuffle = "Shuffle",
    .s_info = "Info",
    .s_lyrics = "Lyrics",
    .s_empty_music = "Add music files to:",
    .s_no_favorite = "No favorites yet",
};
lang_t *curr_lang = &g_lang_en;

void odroid_overlay_draw_fill_rect(int x, int y, int width, int height, uint16_t color)
{
    if (width <= 0 || height <= 0)
        return;
    uint16_t *fb = (uint16_t *)lcd_get_active_buffer();
    if (!fb)
        return;
    for (int j = 0; j < height; j++) {
        int yy = y + j;
        if (yy < 0 || yy >= GW_LCD_HEIGHT)
            continue;
        uint16_t *row = fb + yy * GW_LCD_WIDTH;
        for (int i = 0; i < width; i++) {
            int xx = x + i;
            if (xx >= 0 && xx < GW_LCD_WIDTH)
                row[xx] = color;
        }
    }
}

/* --- rg_storage_scandir via FatFs (no heap) ------------------------------- */
static int scandir_iname(const char *a, const char *b)
{
    while (*a && *b) {
        unsigned char ca = (unsigned char)*a++;
        unsigned char cb = (unsigned char)*b++;
        if (ca >= 'A' && ca <= 'Z')
            ca = (unsigned char)(ca + 32);
        if (cb >= 'A' && cb <= 'Z')
            cb = (unsigned char)(cb + 32);
        if (ca != cb)
            return (int)ca - (int)cb;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static int scandir_cmp(const void *a, const void *b)
{
    const rg_scandir_t *xa = a, *xb = b;
    if (xa->is_dir != xb->is_dir)
        return (int)xb->is_dir - (int)xa->is_dir;
    return scandir_iname(xa->basename, xb->basename);
}

#define SCANDIR_SORT_MAX 96

static bool scandir_emit(rg_scandir_cb_t *callback, void *arg, uint32_t flags,
                         rg_scandir_t *ent, bool *ok)
{
    int ret = callback(ent, arg);
    if (ret == RG_SCANDIR_STOP)
        return false;
    if (ret == RG_SCANDIR_SKIP)
        return true;
    if ((flags & RG_SCANDIR_RECURSIVE) && ent->is_dir) {
        if (!rg_storage_scandir(ent->path, callback, arg, flags))
            *ok = false;
    }
    return true;
}

bool rg_storage_scandir(const char *path, rg_scandir_cb_t *callback, void *arg, uint32_t flags)
{
    if (!path || !callback)
        return false;

    uint32_t types = flags & (RG_SCANDIR_FILES | RG_SCANDIR_DIRS);
    size_t path_len = strlen(path) + 1;
    if (path_len > RG_PATH_MAX - 5)
        return false;

    DIR dir;
    FILINFO fno;
    FRESULT fr = f_opendir(&dir, path);
    if (fr != FR_OK) {
        printf("scandir: opendir(%s) -> %d\n", path, (int)fr);
        return false;
    }

    static rg_scandir_t sortbuf[SCANDIR_SORT_MAX];
    int n = 0;
    bool sorting = (flags & RG_SCANDIR_SORT) != 0;
    bool flushed = false;
    bool ok = true;
    bool stop = false;

    while (!stop) {
        wdog_refresh();
        if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0)
            break;
        if (fno.fname[0] == '.' && (!fno.fname[1] || fno.fname[1] == '.'))
            continue;
        if (path_len + strlen(fno.fname) >= RG_PATH_MAX)
            continue;

        bool is_dir = (fno.fattrib & AM_DIR) != 0;
        bool is_file = !is_dir;
        if (!((is_dir && types != RG_SCANDIR_FILES) || (is_file && types != RG_SCANDIR_DIRS)))
            continue;

        rg_scandir_t tmp;
        memset(&tmp, 0, sizeof(tmp));
        memcpy(tmp.path, path, path_len - 1);
        tmp.path[path_len - 1] = '/';
        memcpy(tmp.path + path_len, fno.fname, strlen(fno.fname) + 1);
        tmp.basename = tmp.path + path_len;
        tmp.dirname = path;
        tmp.is_dir = is_dir;
        tmp.is_file = is_file;
        tmp.size = (size_t)fno.fsize;
        tmp.mtime = fno.ftime;

        if (sorting && n < SCANDIR_SORT_MAX) {
            sortbuf[n++] = tmp;
            sortbuf[n - 1].basename = sortbuf[n - 1].path + path_len;
            continue;
        }

        if (sorting && !flushed) {
            qsort(sortbuf, (size_t)n, sizeof(sortbuf[0]), scandir_cmp);
            for (int i = 0; i < n; i++) {
                wdog_refresh();
                sortbuf[i].basename = sortbuf[i].path + path_len;
                if (!scandir_emit(callback, arg, flags, &sortbuf[i], &ok)) {
                    stop = true;
                    break;
                }
            }
            n = 0;
            flushed = true;
            sorting = false;
            if (stop)
                break;
        }

        if (!scandir_emit(callback, arg, flags, &tmp, &ok))
            stop = true;
    }
    f_closedir(&dir);

    if (!stop && sorting && n > 0) {
        qsort(sortbuf, (size_t)n, sizeof(sortbuf[0]), scandir_cmp);
        for (int i = 0; i < n; i++) {
            wdog_refresh();
            sortbuf[i].basename = sortbuf[i].path + path_len;
            if (!scandir_emit(callback, arg, flags, &sortbuf[i], &ok))
                break;
        }
    }
    return ok;
}

