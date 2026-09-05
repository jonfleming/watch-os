set(VR_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/../../voice-recorder/main")
set(OAI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../openai-realtime-embedded/src")

idf_component_register(
    SRCS
        "app_main.c"
        "app_select.c"
        "start_menu.c"
        "${VR_MAIN}/app_main.c"
        "${VR_MAIN}/wav.c"
        "${VR_MAIN}/filesystem_manager.c"
        "${VR_MAIN}/audio_commons.c"
        "${VR_MAIN}/audio_recorder.c"
        "${VR_MAIN}/audio_player.c"
        "${VR_MAIN}/button_manager.c"
        "${VR_MAIN}/ui_manager.c"
        "${VR_MAIN}/state_machine.c"
        "${VR_MAIN}/power_manager.c"
        "${OAI_SRC}/main.cpp"
        "${OAI_SRC}/webrtc.cpp"
        "${OAI_SRC}/http.cpp"
        "${OAI_SRC}/lcd.cpp"
        "${OAI_SRC}/wifi_config.cpp"
        "${OAI_SRC}/media.cpp"
        "${OAI_SRC}/aipi_lite_config.c"
        "${OAI_SRC}/es8311.c"
        "${OAI_SRC}/battery.cpp"
    INCLUDE_DIRS
        "."
        "${VR_MAIN}"
        "${OAI_SRC}"
    REQUIRES
        driver
        esp_wifi
        esp_netif
        nvs_flash
        peer
        esp_psram
        esp-libopus
        esp_http_client
        esp_https_server
        json
        esp_lcd
        esp_lcd_touch
        esp_lvgl_port
        lvgl
        esp_codec_dev
        esp_driver_gpio
        esp_driver_i2s
        sdmmc
        fatfs
        spiffs
        wpa_supplicant
        esp_timer
        esp_adc
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=format-truncation
    -Wno-error=format-overflow
)

# Same warning suppressions the standalone assistant applies to vendored libs.
idf_component_get_property(lib peer COMPONENT_LIB)
if(lib)
    target_compile_options(${lib} PRIVATE -Wno-error=restrict)
    target_compile_options(${lib} PRIVATE -Wno-error=stringop-truncation)
endif()

idf_component_get_property(lib srtp COMPONENT_LIB)
if(lib)
    target_compile_options(${lib} PRIVATE -Wno-error=incompatible-pointer-types)
endif()

idf_component_get_property(lib esp-libopus COMPONENT_LIB)
if(lib)
    target_compile_options(${lib} PRIVATE -Wno-error=maybe-uninitialized)
    target_compile_options(${lib} PRIVATE -Wno-error=stringop-overread)
endif()
