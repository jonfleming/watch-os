cmake_minimum_required(VERSION 3.19)

# Combined firmware: Start Menu + Voice Recorder + Voice Assistant.
# Boards (exactly one Waveshare BSP; they export the same bsp_* symbols):
#   idf.py build                                 → 2.06 (default)
#   idf.py -DWAVESHARE_AMOLED_1_8_BOARD=ON build → 1.8
#   idf.py -DWAVESHARE_AMOLED_2_06_BOARD=OFF build → 1.8 (same)
if(NOT DEFINED IDF_TARGET)
  set(IDF_TARGET esp32s3)
endif()

set(OAI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../openai-realtime-embedded")
set(VR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../voice-recorder")

# Exactly one Waveshare BSP may be linked (they export the same bsp_* symbols).
option(WAVESHARE_AMOLED_2_06_BOARD "Build for Waveshare ESP32-S3 Touch AMOLED 2.06" ON)
option(WAVESHARE_AMOLED_1_8_BOARD "Build for Waveshare ESP32-S3 Touch AMOLED 1.8" OFF)
if(WAVESHARE_AMOLED_1_8_BOARD OR NOT WAVESHARE_AMOLED_2_06_BOARD)
  add_compile_definitions(WAVESHARE_AMOLED_2_06_BOARD=0)
  add_compile_definitions(WAVESHARE_AMOLED_1_8_BOARD=1)
  add_compile_definitions(AIPI_LITE_BOARD=0)
  add_compile_definitions(WAVESHARE_BSP_BOARD=1)
  set(ENV{FREEBUFF_BOARD} "waveshare_amoled_1_8")
  set(ENV{VOICE_RECORDER_BOARD} "waveshare_amoled_1_8")
  set(SDKCONFIG_DEFAULTS "sdkconfig.defaults;sdkconfig.waveshare_amoled_1_8")
  message(STATUS "watch-os: Waveshare AMOLED 1.8 (368x448, 16 MB, USB-Serial-JTAG)")
else()
  add_compile_definitions(WAVESHARE_AMOLED_2_06_BOARD=1)
  add_compile_definitions(WAVESHARE_AMOLED_1_8_BOARD=0)
  add_compile_definitions(AIPI_LITE_BOARD=0)
  add_compile_definitions(WAVESHARE_BSP_BOARD=1)
  set(ENV{FREEBUFF_BOARD} "waveshare_amoled_2_06")
  set(ENV{VOICE_RECORDER_BOARD} "waveshare_amoled_2_06")
  set(SDKCONFIG_DEFAULTS "sdkconfig.defaults;sdkconfig.waveshare_amoled_2_06")
  message(STATUS "watch-os: Waveshare AMOLED 2.06 (410x502, 32 MB QIO)")
endif()

add_compile_definitions(WATCH_OS_SHELL=1)
add_compile_definitions(OPENAI_REALTIMEAPI="https://speech.fleming.ai/v1/realtime")

# TURN credentials live in the assistant project's gitignored privateConfig.json.
if(EXISTS "${OAI_ROOT}/privateConfig.json")
  file(READ "${OAI_ROOT}/privateConfig.json" PV_CFG)
  string(JSON PV_TURN_USERNAME ERROR_VARIABLE PV_TURN_USERNAME_ERR GET "${PV_CFG}" turn_username)
  if(PV_TURN_USERNAME_ERR)
    set(PV_TURN_USERNAME "")
    set(PV_TURN_PASSWORD "")
    set(PV_TURN_SERVER "")
  else()
    string(JSON PV_TURN_PASSWORD GET "${PV_CFG}" turn_password)
    string(JSON PV_TURN_SERVER ERROR_VARIABLE PV_TURN_SERVER_ERR GET "${PV_CFG}" turn_server)
    if(PV_TURN_SERVER_ERR)
      set(PV_TURN_SERVER "turn:turn.techion.net:3478")
    endif()
  endif()
else()
  set(PV_TURN_USERNAME "")
  set(PV_TURN_PASSWORD "")
  set(PV_TURN_SERVER "")
endif()
if(PV_TURN_SERVER)
  add_compile_definitions(TURN_SERVER_URL="${PV_TURN_SERVER}")
endif()
add_compile_definitions(TURN_USERNAME="${PV_TURN_USERNAME}")
add_compile_definitions(TURN_PASSWORD="${PV_TURN_PASSWORD}")

# libpeer / libsrtp / opus stay in the assistant tree; relative paths inside
# those components (e.g. peer → ../../deps/libpeer) still resolve.
list(APPEND EXTRA_COMPONENT_DIRS
  "${OAI_ROOT}/components/srtp"
  "${OAI_ROOT}/components/peer"
  "${OAI_ROOT}/components/esp-libopus"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(watch-os)
