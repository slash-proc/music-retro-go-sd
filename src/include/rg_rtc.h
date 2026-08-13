#pragma once
#include <stdint.h>
#include <time.h>

/* Prefer time()+localtime(); these remain for the minute-tick redraw. */
static inline uint8_t GW_GetCurrentMinute(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return tm ? (uint8_t)tm->tm_min : 0;
}
