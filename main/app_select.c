#include "app_select.h"

#include "esp_attr.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "app_select";

#define LAUNCH_MAGIC 0xA11A0C00u

RTC_NOINIT_ATTR static uint32_t s_launch_magic;
RTC_NOINIT_ATTR static uint32_t s_launch_app;

app_id_t app_select_pending(void)
{
    if (esp_reset_reason() == ESP_RST_SW && s_launch_magic == LAUNCH_MAGIC) {
        ESP_LOGI(TAG, "SW restart launch token: app %lu", (unsigned long)s_launch_app);
        return (app_id_t)s_launch_app;
    }
    ESP_LOGI(TAG, "No launch token (reset %d) — start menu", (int)esp_reset_reason());
    return APP_ID_MENU;
}

void app_select_launch(app_id_t id)
{
    s_launch_magic = LAUNCH_MAGIC;
    s_launch_app = (uint32_t)id;
    ESP_LOGI(TAG, "Launching app %d", (int)id);
    esp_restart();
}

void app_select_request_menu(void)
{
    app_select_launch(APP_ID_MENU);
}
