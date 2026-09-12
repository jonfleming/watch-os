#include "watch_display.h"

#include "board.h"
#include "esp_log.h"
#include "lcd.h"

static const char *TAG = "watch_disp";

lv_display_t *watch_display_start(void)
{
#if defined(WAVESHARE_AMOLED_1_8_BOARD) && WAVESHARE_AMOLED_1_8_BOARD
    // Do not call bsp_display_start() here. Waveshare BSP 2.0.3 ignores a
    // failed touch probe and hands NULL to lvgl_port_add_touch(), which
    // LoadProhibited-panics because this firmware builds with assertions
    // off. init_lvgl() pulses the TCA9554 (touch/panel power on V1.0) and
    // continues without touch if the probe still fails.
    if (init_lvgl() != ESP_OK) {
        ESP_LOGE(TAG, "init_lvgl failed");
        return NULL;
    }
    return disp_handle;
#else
    return bsp_display_start();
#endif
}
