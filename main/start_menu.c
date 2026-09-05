#include "start_menu.h"

#include "app_select.h"
#include "board.h"
#include "bsp/display.h"
#include "button_manager.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "power_manager.h"

static const char *TAG = "start_menu";

static int s_highlight = 0;
static lv_obj_t *s_cards[2];
static lv_obj_t *s_scr;

static void style_card(lv_obj_t *card, bool selected)
{
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(card, selected ? 3 : 1, 0);
    lv_obj_set_style_border_color(card,
        selected ? lv_color_hex(0x00aa44) : lv_color_hex(0x333333), 0);
}

static void launch(app_id_t id)
{
    ESP_LOGI(TAG, "Start menu selected app %d", (int)id);
    app_select_launch(id);
}

static void recorder_cb(lv_event_t *e)
{
    (void)e;
    launch(APP_ID_RECORDER);
}

static void assistant_cb(lv_event_t *e)
{
    (void)e;
    launch(APP_ID_ASSISTANT);
}

static lv_obj_t *make_card(lv_obj_t *parent, const char *title, const char *sub,
                           int y, lv_event_cb_t cb)
{
    const int w = BOARD_LCD_H_RES - 40;
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, w, 110);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x333333), 0);
    lv_obj_set_style_pad_all(card, 16, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(card, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *t = lv_label_create(card);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, lv_color_white(), 0);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_20, 0);
    lv_obj_align(t, LV_ALIGN_TOP_LEFT, 0, 8);

    lv_obj_t *s = lv_label_create(card);
    lv_label_set_text(s, sub);
    lv_obj_set_style_text_color(s, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(s, LV_ALIGN_BOTTOM_LEFT, 0, -8);
    return card;
}

static void refresh_highlight(void)
{
    if (!bsp_display_lock(100)) {
        return;
    }
    style_card(s_cards[0], s_highlight == 0);
    style_card(s_cards[1], s_highlight == 1);
    bsp_display_unlock();
}

void start_menu_run(void)
{
    ESP_LOGI(TAG, "Start menu on %s", board_name());

    esp_log_level_t i2c_log = esp_log_level_get("i2c.master");
    esp_log_level_set("i2c.master", ESP_LOG_NONE);
    lv_display_t *disp = bsp_display_start();
    esp_log_level_set("i2c.master", i2c_log);
    if (!disp) {
        ESP_LOGE(TAG, "Display init failed");
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
    }
    bsp_display_brightness_set(100);
    vTaskDelay(pdMS_TO_TICKS(80));

    if (power_manager_init() != ESP_OK) {
        ESP_LOGW(TAG, "power_manager init failed — PWR toggle disabled");
    }
    button_manager_init();

    if (!bsp_display_lock(200)) {
        ESP_LOGE(TAG, "LVGL lock failed");
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
    }

    s_scr = lv_scr_act();
    lv_obj_set_style_bg_color(s_scr, lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(s_scr);
    lv_label_set_text(title, "Start");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t *hint = lv_label_create(s_scr);
    lv_label_set_text(hint, "Tap an app to launch");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 64);

    s_cards[0] = make_card(s_scr, "Voice Recorder", "Record and play WAVs",
                           110, recorder_cb);
    s_cards[1] = make_card(s_scr, "Voice Assistant", "Realtime conversation",
                           238, assistant_cb);
    style_card(s_cards[0], true);

    lv_obj_t *footer = lv_label_create(s_scr);
    lv_label_set_text(footer, "A short: next   A long: open\nLong-press A in an app to return");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_align(footer, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -24);

    bsp_display_unlock();

    while (1) {
        if (button_a_was_pressed()) {
            s_highlight = 1 - s_highlight;
            ESP_LOGI(TAG, "Highlight %d", s_highlight);
            refresh_highlight();
        }
        if (button_a_was_long_pressed()) {
            launch(s_highlight == 0 ? APP_ID_RECORDER : APP_ID_ASSISTANT);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
