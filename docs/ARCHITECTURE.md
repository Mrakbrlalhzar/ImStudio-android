# Architecture

This project re-implements ImStudio's editor UI natively on Android using
Jetpack Compose while reusing the upstream C++ core (`ims_buffer`, `ims_generator`,
`ims_object`, plus ImGui headers).

Layers:

- `android/app/src/main/cpp/vendor/imstudio/` — verbatim copy of upstream C++ core
- `android/app/src/main/cpp/vendor/imgui/` — ImGui core, no backends
- `android/app/src/main/cpp/vendor/fmt/` — header-only fmt
- `android/app/src/main/cpp/src/engine/buffer_accessor.*` — wraps `ImStudio::BufferWindow`
- `android/app/src/main/cpp/src/engine/code_generator.*` — wraps `ImStudio::Recreate`
- `android/app/src/main/cpp/src/jni_bridge.cpp` — JNI surface
- `android/app/src/main/java/com/raais/imstudio/engine/` — Kotlin state layer
- `android/app/src/main/java/com/raais/imstudio/ui/` — Jetpack Compose UI

## Sync model
Files under `vendor/` are overwritten from upstream on every workflow run.
All other files are generated only if missing, so your edits persist.
