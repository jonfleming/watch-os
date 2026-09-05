# watch-os — Start Menu firmware

One ESP-IDF binary for the Waveshare ESP32-S3 Touch AMOLED 2.06 that hosts:

1. **Start Menu** (boot)
2. **Voice Recorder** (`../voice-recorder`)
3. **Voice Assistant** (`../openai-realtime-embedded`)

## Why one binary, and why restart?

Both apps already share the same BSP (display, I2C, ES8311+ES7210, GPIO0). Flash is not the limit — the recorder is ~700 KB and the assistant is well under the 8 MB factory partition.

The hard part is *leaving* an app. The assistant’s WebRTC/libpeer loop, Opus task, WiFi portal, and I2S/codec handles have no clean teardown (a disconnect already calls `esp_restart()`). The BSP audio/display init functions are one-shot.

So the shell does **not** tear hardware down in-process. Selecting an app writes a launch token to RTC memory and reboots. The chosen app then does its normal init on a fresh chip. Long-press BOOT (button A, 1.5 s) writes “menu” and reboots again.

A software restart (including the assistant reconnecting after a dropped peer) keeps the token, so you stay in that app. A power-cycle loses RTC memory and shows the Start Menu.

`bmorcelli/Launcher` is the fallback if you later want third-party `.bin` files from the microSD. It is an Arduino loader and does not match these two ESP-IDF trees, so it is not used here.

## Adding another app

1. Give it a `*_run()` that never returns (same pattern as `voice_recorder_run` / `voice_assistant_run`).
2. Add an `APP_ID_*` in `main/app_select.h`.
3. Add a Start Menu card in `main/start_menu.c`.
4. Call `app_select_request_menu()` from a long-press of button A (weak symbol; this project provides the strong one).
5. Add its sources to `main/CMakeLists.txt`.

## Build

ESP-IDF v5.5.5. From an IDF PowerShell:

```powershell
cd C:\Projects\ai-voice-assistant\watch-os
idf.py build
idf.py -p COMx flash monitor
```

TURN / API settings are read from `../openai-realtime-embedded/privateConfig.json` at CMake time (same as the standalone assistant).

## Controls

| Where | Input | Action |
|---|---|---|
| Start Menu | Tap a card | Launch that app |
| Start Menu | Short BOOT (A) | Move highlight |
| Start Menu | Long BOOT (A) | Launch highlighted app |
| Either app | Long BOOT (A), 1.5 s | Return to Start Menu |
| Voice Recorder | Short BOOT (A) | Start / stop recording |
| Voice Assistant | Short BOOT (A) | Pause / resume (interrupt) |
| Anywhere | Short PWR | Display on/off (AXP2101) |
| Anywhere | Long PWR (~6 s) | Hardware power off |
