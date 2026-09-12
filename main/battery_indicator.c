#include "battery_indicator.h"

#include "battery.h"
#include "board.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#include <stdio.h>

static const char *TAG = "batt_ind";

// Compact status-bar version of the assistant's green charge bar. Self-contained
// so it stays readable on both the dark start-menu/recorder screens and the
// assistant's white canvas.
#define BATTERY_BAR_W  90
#define BATTERY_BAR_H  16

static lv_obj_t *s_bar;
static bool s_started;

static void ensure_widget(void)
{
    if (s_bar != NULL && lv_obj_is_valid(s_bar)) {
        return;
    }

    lv_obj_t *top = lv_layer_top();
    s_bar = lv_bar_create(top);
    lv_obj_set_size(s_bar, BATTERY_BAR_W, BATTERY_BAR_H);
    lv_bar_set_range(s_bar, 0, 100);
    lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);
    // Inset past the 2.06 AMOLED corner radius so the pill stays on-panel.
    lv_obj_align(s_bar, LV_ALIGN_TOP_RIGHT, -22, 18);
    lv_obj_clear_flag(s_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_bar, lv_color_hex(0xDADADA), LV_PART_MAIN);
    lv_obj_set_style_radius(s_bar, BATTERY_BAR_H / 2, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_bar, lv_color_hex(0x2E7D32), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_bar, BATTERY_BAR_H / 2 - 1, LV_PART_INDICATOR);

    lv_obj_t *label = lv_label_create(s_bar);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_label_set_text(label, "");
    lv_obj_center(label);
}

static void apply_percent(int pct)
{
    ensure_widget();
    if (pct < 0 || pct > 100) {
        lv_obj_add_flag(s_bar, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(s_bar, LV_OBJ_FLAG_HIDDEN);
    lv_bar_set_value(s_bar, pct, LV_ANIM_OFF);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", pct);
    lv_label_set_text(lv_obj_get_child(s_bar, 0), buf);
}

static void battery_indicator_task(void *arg)
{
    (void)arg;
    if (oai_battery_init() != ESP_OK) {
        ESP_LOGW(TAG, "Battery monitor unavailable — indicator hidden");
        s_started = false;
        vTaskDelete(NULL);
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(400));
    int last_pct = -2;
    while (1) {
        int pct = oai_battery_get_percent();
        if (pct != last_pct || s_bar == NULL || !lv_obj_is_valid(s_bar)) {
            if (bsp_display_lock(100)) {
                apply_percent(pct);
                bsp_display_unlock();
                last_pct = pct;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void battery_indicator_start(void)
{
    if (s_started) {
        return;
    }
    s_started = true;
    if (xTaskCreate(battery_indicator_task, "batt_ind", 4096, NULL, 2, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start battery indicator task");
        s_started = false;
    }
}
