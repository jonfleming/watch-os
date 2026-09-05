#include "app_select.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "start_menu.h"
#include "voice_recorder.h"
#include "wifi_config.h"

void voice_assistant_run(void);

static const char *TAG = "watch-os";

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // UTC survives software reset; TZ does not. Re-apply so recorder
    // filenames stay local after a visit to the assistant settings page.
    wifi_config_apply_saved_timezone();

    app_id_t app = app_select_pending();
    ESP_LOGI(TAG, "Boot → %s",
             app == APP_ID_RECORDER ? "Voice Recorder" :
             app == APP_ID_ASSISTANT ? "Voice Assistant" : "Start Menu");

    switch (app) {
    case APP_ID_RECORDER:
        voice_recorder_run();
        break;
    case APP_ID_ASSISTANT:
        voice_assistant_run();
        break;
    case APP_ID_MENU:
    default:
        start_menu_run();
        break;
    }
}
