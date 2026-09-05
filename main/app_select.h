#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_ID_MENU = 0,
    APP_ID_RECORDER = 1,
    APP_ID_ASSISTANT = 2,
} app_id_t;

// Read the app to run this boot. Honours a software-restart launch token
// (so WebRTC reconnect restarts stay in the assistant). Power-on, panic,
// and brownout fall back to the start menu.
app_id_t app_select_pending(void);

// Store `id` in RTC memory and reboot into it.
void app_select_launch(app_id_t id);

// Long-press home: reboot into the start menu. Also the strong definition
// of the weak symbol the two apps call.
void app_select_request_menu(void);

#ifdef __cplusplus
}
#endif
