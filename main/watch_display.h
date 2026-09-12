#pragma once

#include "lvgl.h"

// Bring up panel + LVGL. On the 1.8 V1.0 this pulses the TCA9554 and never
// passes a NULL touch handle into LVGL (stock BSP 2.0.3 panics if the probe
// fails). On the 2.06 this is bsp_display_start().
lv_display_t *watch_display_start(void);
