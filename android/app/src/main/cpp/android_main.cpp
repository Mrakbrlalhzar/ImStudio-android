#include <android/log.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include "widget_model.h"

#define LOG_TAG "ImGuiEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

enum class AppMode { Editor, Test };
struct TouchPoint { int32_t id; float x, y; };

struct EngineState {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    bool eglReady = false;
    bool imguiReady = false;
    int width = 0, height = 0;
    bool animating = true;
    WidgetModel model;
    AppMode mode = AppMode::Editor;
    std::vector<TouchPoint> activeTouches;
    float toastTimer = 0.0f;
    char toastText[128] = "";
};
static EngineState g_Engine;

static void show_toast(const char* msg) {
    std::strncpy(g_Engine.toastText, msg, sizeof(g_Engine.toastText) - 1);
    g_Engine.toastText[sizeof(g_Engine.toastText) - 1] = 0;
    g_Engine.toastTimer = 2.0f;
}

// EGL init/destroy, ImGui init/destroy, render_widget, render_inspector,
// render_editor_screen, render_test_screen, render_toast,
// render_frame, handle_app_cmd, handle_input, android_main
// ... (این بخش‌ها دقیقاً مشابه کدهای قبلی هستند و برای جلوگیری از طولانی شدن، اینجا تکرار نمی‌شوند) ...

// !!! نکته مهم: کل منطق رندر و مدیریت ورودی باید از YAML قبلی کپی شود.
// فقط بخش CMakeLists.txt تغییر کرده است.

