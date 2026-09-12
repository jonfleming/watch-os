#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// System-level battery chrome for watch-os. Lives on LVGL's top layer so it
// survives app screen rebuilds. Safe to call more than once; no-op if the
// monitor is already running. Hidden when no battery is present.
void battery_indicator_start(void);

#ifdef __cplusplus
}
#endif
