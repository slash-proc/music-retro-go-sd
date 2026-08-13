#pragma once
#include <stdbool.h>
/* Clock-alarm ringing while Music is open is firmware-only; no-op here. */
static inline bool rg_alarm_poll(void) { return false; }
