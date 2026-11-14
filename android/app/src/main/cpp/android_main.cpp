#include <android/log.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <jni.h>
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

// ============================================================
// 软键盘 JNI 桥接 (基于 issue #408 的方案)
// ============================================================
static JavaVM* g_JavaVM = nullptr;
static jobject g_ActivityObject = nullptr;

static void ShowSoftKeyboard() {
    if (!g_JavaVM || !g_ActivityObject) return;
    JNIEnv* env = nullptr;
    g_JavaVM->AttachCurrentThread(&env, nullptr);
    if (!env) return;
    jclass clazz = env->GetObjectClass(g_ActivityObject);
    jmethodID method = env->GetMethodID(clazz, "ShowAndroidKeyboard", "()V");
    if (method) env->CallVoidMethod(g_ActivityObject, method);
    env->DeleteLocalRef(clazz);
}

// ============================================================
// 状态结构
// ============================================================
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
    bool wantKeyboard = false;
};
static EngineState g_Engine;

static void show_toast(const char* msg) {
    std::strncpy(g_Engine.toastText, msg, sizeof(g_Engine.toastText) - 1);
    g_Engine.toastText[sizeof(g_Engine.toastText) - 1] = 0;
    g_Engine.toastTimer = 2.0f;
}

// ============================================================
// EGL 初始化
// ============================================================
static bool init_egl(android_app* app) {
    if (g_Engine.eglReady) return true;
    if (!app->window) return false;
    g_Engine.width = ANativeWindow_getWidth(app->window);
    g_Engine.height = ANativeWindow_getHeight(app->window);

    g_Engine.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_Engine.display == EGL_NO_DISPLAY) return false;
    if (!eglInitialize(g_Engine.display, nullptr, nullptr)) return false;

    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_NONE
    };
    EGLConfig config; EGLint numConfigs;
    if (!eglChooseConfig(g_Engine.display, attribs, &config, 1, &numConfigs) || numConfigs == 0) return false;

    g_Engine.surface = eglCreateWindowSurface(g_Engine.display, config, app->window, nullptr);
    if (g_Engine.surface == EGL_NO_SURFACE) return false;

    const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    g_Engine.context = eglCreateContext(g_Engine.display, config, EGL_NO_CONTEXT, ctxAttribs);
    if (g_Engine.context == EGL_NO_CONTEXT) return false;

    if (!eglMakeCurrent(g_Engine.display, g_Engine.surface, g_Engine.surface, g_Engine.context)) return false;

    g_Engine.eglReady = true;
    LOGI("EGL ready %dx%d", g_Engine.width, g_Engine.height);
    return true;
}

static void destroy_egl_all() {
    if (g_Engine.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_Engine.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_Engine.context != EGL_NO_CONTEXT) { eglDestroyContext(g_Engine.display, g_Engine.context); g_Engine.context = EGL_NO_CONTEXT; }
        if (g_Engine.surface != EGL_NO_SURFACE) { eglDestroySurface(g_Engine.display, g_Engine.surface); g_Engine.surface = EGL_NO_SURFACE; }
        eglTerminate(g_Engine.display);
        g_Engine.display = EGL_NO_DISPLAY;
    }
    g_Engine.eglReady = false;
}

// ============================================================
// ImGui 初始化
// ============================================================
static bool init_imgui(android_app* app) {
    if (g_Engine.imguiReady) return true;
    if (!app->window) return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(8, 8);

    ImGui_ImplAndroid_Init(app->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    g_Engine.imguiReady = true;
    LOGI("ImGui ready");
    return true;
}

static void destroy_imgui() {
    if (!g_Engine.imguiReady) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    g_Engine.imguiReady = false;
}

// ============================================================
// 渲染控件 (编辑器 / 测试模式共用)
// ============================================================
static void render_widget(Widget& w, bool editorMode) {
    ImGui::PushID(w.id);
    ImGui::SetCursorPos(ImVec2(w.x, w.y));
    const ImVec2 size = (w.h > 0) ? ImVec2(w.w, w.h) : ImVec2(w.w, 0);

    switch (w.kind) {
        case WidgetKind::Button:      ImGui::Button(w.label, size); break;
        case WidgetKind::Checkbox:    ImGui::Checkbox(w.label, &w.checked); break;
        case WidgetKind::Radio:       ImGui::RadioButton(w.label, w.checked); break;
        case WidgetKind::SliderFloat: ImGui::SetNextItemWidth(w.w); ImGui::SliderFloat(w.label, &w.floatValue, w.minValue, w.maxValue); break;
        case WidgetKind::SliderInt:   ImGui::SetNextItemWidth(w.w); ImGui::SliderInt(w.label, &w.intValue, (int)w.minValue, (int)w.maxValue); break;
        case WidgetKind::DragFloat:   ImGui::SetNextItemWidth(w.w); ImGui::DragFloat(w.label, &w.floatValue, 0.01f, w.minValue, w.maxValue); break;
        case WidgetKind::InputText:   ImGui::SetNextItemWidth(w.w); ImGui::InputText(w.label, w.valueBuf, sizeof(w.valueBuf)); break;
        case WidgetKind::Combo: {
            ImGui::SetNextItemWidth(w.w);
            std::vector<const char*> items;
            for (auto& s : w.comboItems) items.push_back(s.c_str());
            if (!items.empty()) ImGui::Combo(w.label, &w.comboIndex, items.data(), (int)items.size());
            break;
        }
        case WidgetKind::Text:         ImGui::TextUnformatted(w.label); break;
        case WidgetKind::ProgressBar:  ImGui::ProgressBar(w.floatValue, ImVec2(w.w, 0)); break;
        case WidgetKind::Separator:    ImGui::Separator(); break;
        case WidgetKind::SameLine:     ImGui::SameLine(); break;
        case WidgetKind::Spacing:      ImGui::Spacing(); break;
    }

    // 软键盘触发
    if (ImGui::IsItemActive() && (w.kind == WidgetKind::InputText)) {
        g_Engine.wantKeyboard = true;
    }

    if (editorMode) {
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
            g_Engine.model.selectedId = w.id;

        if (g_Engine.model.editMode && ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
            ImVec2 d = ImGui::GetMouseDragDelta(0);
            if (std::fabs(d.x) > 1.5f || std::fabs(d.y) > 1.5f) {
                w.x += d.x; w.y += d.y;
                ImGui::ResetMouseDragDelta(0);
            }
        }

        if (g_Engine.model.selectedId == w.id) {
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            ImVec2 mn = ImGui::GetItemRectMin();
            ImVec2 mx = ImGui::GetItemRectMax();
            fg->AddRect(ImVec2(mn.x-2, mn.y-2), ImVec2(mx.x+2, mx.y+2),
                        IM_COL32(0, 220, 255, 255), 4.0f, 0, 2.0f);
        }
    }
    ImGui::PopID();
}

// ... [Inspector、Editor 界面、Test 界面、触摸可视化、主循环等代码，与之前版本相同，此处为简洁省略] ...
// 完整代码请参考前文 YAML 中的对应部分，或直接复制使用。
// 关键点：Test 模式使用 glClearColor(0,0,0,1) 纯黑背景，并通过 ImGui::GetForegroundDrawList() 绘制触摸点。

// ============================================================
// 生命周期与输入处理
// ============================================================
static void handle_app_cmd(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window) { if (init_egl(app)) { init_imgui(app); g_Engine.animating = true; } }
            break;
        case APP_CMD_TERM_WINDOW:
            destroy_imgui();
            if (g_Engine.display != EGL_NO_DISPLAY) {
                eglMakeCurrent(g_Engine.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (g_Engine.surface != EGL_NO_SURFACE) { eglDestroySurface(g_Engine.display, g_Engine.surface); g_Engine.surface = EGL_NO_SURFACE; }
            }
            g_Engine.eglReady = false;
            g_Engine.animating = false;
            break;
        case APP_CMD_GAINED_FOCUS: g_Engine.animating = true; break;
        case APP_CMD_LOST_FOCUS:  g_Engine.animating = false; break;
        case APP_CMD_DESTROY:     destroy_imgui(); destroy_egl_all(); break;
    }
}

static int32_t handle_input(android_app* app, AInputEvent* event) {
    if (!g_Engine.imguiReady) return 0;

    int32_t type = AInputEvent_getType(event);

    // 物理返回键处理
    if (type == AINPUT_EVENT_TYPE_KEY) {
        int32_t keyCode = AKeyEvent_getKeyCode(event);
        int32_t action = AKeyEvent_getAction(event);
        if (keyCode == AKEYCODE_BACK && action == AKEY_EVENT_ACTION_UP) {
            if (g_Engine.mode == AppMode::Test) {
                g_Engine.mode = AppMode::Editor;
                show_toast("Returned to editor");
                return 1;
            }
            return 0;
        }
    }

    // 触摸点追踪 (用于测试模式可视化)
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t actionMasked = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        size_t pointerCount = AMotionEvent_getPointerCount(event);

        if (actionMasked == AMOTION_EVENT_ACTION_DOWN ||
            actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN ||
            actionMasked == AMOTION_EVENT_ACTION_MOVE) {
            g_Engine.activeTouches.clear();
            for (size_t i = 0; i < pointerCount; i++) {
                TouchPoint tp;
                tp.id = AMotionEvent_getPointerId(event, i);
                tp.x = AMotionEvent_getX(event, i);
                tp.y = AMotionEvent_getY(event, i);
                g_Engine.activeTouches.push_back(tp);
            }
        } else if (actionMasked == AMOTION_EVENT_ACTION_POINTER_UP ||
                   actionMasked == AMOTION_EVENT_ACTION_UP) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                                   >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            if (actionMasked == AMOTION_EVENT_ACTION_UP) {
                g_Engine.activeTouches.clear();
            } else if ((int32_t)pointerCount > pointerIndex) {
                int32_t upId = AMotionEvent_getPointerId(event, pointerIndex);
                g_Engine.activeTouches.erase(
                    std::remove_if(g_Engine.activeTouches.begin(), g_Engine.activeTouches.end(),
                        [upId](const TouchPoint& tp) { return tp.id == upId; }),
                    g_Engine.activeTouches.end());
            }
        } else if (actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
            g_Engine.activeTouches.clear();
        }
    }

    return ImGui_ImplAndroid_HandleInputEvent(event);
}

void android_main(android_app* app) {
    LOGI("android_main started (ImGui Bundle)");
    app->onAppCmd = handle_app_cmd;
    app->onInputEvent = handle_input;
    g_Engine.model.initDemo();

    // 初始化 JNI 环境 (用于软键盘)
    app->activity->vm->AttachCurrentThread(&g_JavaVM, nullptr);
    g_ActivityObject = app->activity->clazz;

    while (true) {
        int events;
        android_poll_source* source;
        while (ALooper_pollAll(g_Engine.animating ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
            if (app->destroyRequested != 0) {
                destroy_imgui(); destroy_egl_all();
                return;
            }
        }

        // 处理软键盘请求
        if (g_Engine.wantKeyboard) {
            ShowSoftKeyboard();
            g_Engine.wantKeyboard = false;
        }

        if (g_Engine.animating && g_Engine.eglReady && g_Engine.imguiReady) render_frame();
    }
}
