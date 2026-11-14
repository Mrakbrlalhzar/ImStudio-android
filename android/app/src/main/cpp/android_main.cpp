#include <android/log.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <cmath>
#include <cstring>

#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

#include "widget_model.h"

#define LOG_TAG "ImGuiEditor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct EngineState {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    bool eglReady = false;
    bool imguiReady = false;
    int width = 0;
    int height = 0;
    bool animating = true;
    WidgetModel model;
};

static EngineState g_Engine;

// ============================================================
// EGL initialization
// ============================================================
static bool init_egl(android_app* app) {
    if (g_Engine.eglReady) return true;

    if (!app->window) {
        LOGE("init_egl: no native window");
        return false;
    }

    g_Engine.width = ANativeWindow_getWidth(app->window);
    g_Engine.height = ANativeWindow_getHeight(app->window);
    LOGI("init_egl: window %dx%d", g_Engine.width, g_Engine.height);

    g_Engine.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_Engine.display == EGL_NO_DISPLAY) { LOGE("no display"); return false; }

    EGLint major, minor;
    if (!eglInitialize(g_Engine.display, &major, &minor)) {
        LOGE("eglInitialize failed");
        return false;
    }
    LOGI("EGL %d.%d", major, minor);

    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };
    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(g_Engine.display, attribs, &config, 1, &numConfigs) || numConfigs == 0) {
        LOGE("no matching config");
        return false;
    }

    g_Engine.surface = eglCreateWindowSurface(g_Engine.display, config, app->window, nullptr);
    if (g_Engine.surface == EGL_NO_SURFACE) { LOGE("create surface failed"); return false; }

    const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    g_Engine.context = eglCreateContext(g_Engine.display, config, EGL_NO_CONTEXT, ctxAttribs);
    if (g_Engine.context == EGL_NO_CONTEXT) { LOGE("create context failed"); return false; }

    if (!eglMakeCurrent(g_Engine.display, g_Engine.surface, g_Engine.surface, g_Engine.context)) {
        LOGE("eglMakeCurrent failed");
        return false;
    }

    g_Engine.eglReady = true;
    LOGI("EGL ready");
    return true;
}

static void destroy_egl_surface_only() {
    if (g_Engine.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_Engine.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_Engine.surface != EGL_NO_SURFACE) {
            eglDestroySurface(g_Engine.display, g_Engine.surface);
            g_Engine.surface = EGL_NO_SURFACE;
        }
    }
    g_Engine.eglReady = false;
    LOGI("EGL surface destroyed (context preserved)");
}

static void destroy_egl_all() {
    if (g_Engine.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_Engine.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_Engine.context != EGL_NO_CONTEXT) {
            eglDestroyContext(g_Engine.display, g_Engine.context);
            g_Engine.context = EGL_NO_CONTEXT;
        }
        if (g_Engine.surface != EGL_NO_SURFACE) {
            eglDestroySurface(g_Engine.display, g_Engine.surface);
            g_Engine.surface = EGL_NO_SURFACE;
        }
        eglTerminate(g_Engine.display);
        g_Engine.display = EGL_NO_DISPLAY;
    }
    g_Engine.eglReady = false;
}

// ============================================================
// ImGui init
// ============================================================
static bool init_imgui(android_app* app) {
    if (g_Engine.imguiReady) return true;
    if (!app->window) return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;   // don't save imgui.ini
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

    // Dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(8, 8);

    ImGui_ImplAndroid_Init(app->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    g_Engine.imguiReady = true;
    LOGI("ImGui initialized");
    return true;
}

static void destroy_imgui() {
    if (!g_Engine.imguiReady) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    g_Engine.imguiReady = false;
    LOGI("ImGui destroyed");
}

// ============================================================
// Widget rendering
// ============================================================
static void render_widget(Widget& w) {
    ImGui::PushID(w.id);
    ImGui::SetCursorPos(ImVec2(w.x, w.y));

    const ImVec2 size = (w.h > 0) ? ImVec2(w.w, w.h) : ImVec2(w.w, 0);

    switch (w.kind) {
        case WidgetKind::Button:
            ImGui::Button(w.label, size);
            break;
        case WidgetKind::Checkbox:
            ImGui::Checkbox(w.label, &w.checked);
            break;
        case WidgetKind::Radio:
            ImGui::RadioButton(w.label, w.checked);
            break;
        case WidgetKind::SliderFloat:
            ImGui::SetNextItemWidth(w.w);
            ImGui::SliderFloat(w.label, &w.floatValue, w.minValue, w.maxValue);
            break;
        case WidgetKind::SliderInt:
            ImGui::SetNextItemWidth(w.w);
            ImGui::SliderInt(w.label, &w.intValue, (int)w.minValue, (int)w.maxValue);
            break;
        case WidgetKind::DragFloat:
            ImGui::SetNextItemWidth(w.w);
            ImGui::DragFloat(w.label, &w.floatValue, 0.01f, w.minValue, w.maxValue);
            break;
        case WidgetKind::InputText:
            ImGui::SetNextItemWidth(w.w);
            ImGui::InputText(w.label, w.valueBuf, sizeof(w.valueBuf));
            break;
        case WidgetKind::Combo: {
            ImGui::SetNextItemWidth(w.w);
            std::vector<const char*> items;
            items.reserve(w.comboItems.size());
            for (auto& s : w.comboItems) items.push_back(s.c_str());
            if (!items.empty()) {
                ImGui::Combo(w.label, &w.comboIndex, items.data(), (int)items.size());
            }
            break;
        }
        case WidgetKind::Text:
            ImGui::TextUnformatted(w.label);
            break;
        case WidgetKind::ProgressBar:
            ImGui::ProgressBar(w.floatValue, ImVec2(w.w, 0));
            break;
        case WidgetKind::Separator:
            ImGui::Separator();
            break;
        case WidgetKind::SameLine:
            ImGui::SameLine();
            break;
        case WidgetKind::Spacing:
            ImGui::Spacing();
            break;
    }

    // ---- Selection & drag-to-move ----
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
        g_Engine.model.selectedId = w.id;
    }

    if (g_Engine.model.editMode && ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        ImVec2 d = ImGui::GetMouseDragDelta(0);
        if (std::fabs(d.x) > 1.5f || std::fabs(d.y) > 1.5f) {
            w.x += d.x;
            w.y += d.y;
            ImGui::ResetMouseDragDelta(0);
        }
    }

    // ---- Highlight selection ----
    if (g_Engine.model.selectedId == w.id) {
        ImDrawList* fg = ImGui::GetForegroundDrawList();
        ImVec2 mn = ImGui::GetItemRectMin();
        ImVec2 mx = ImGui::GetItemRectMax();
        fg->AddRect(ImVec2(mn.x - 2, mn.y - 2), ImVec2(mx.x + 2, mx.y + 2),
                    IM_COL32(0, 220, 255, 255), 4.0f, 0, 2.0f);
    }

    ImGui::PopID();
}

// ============================================================
// Inspector panel (ImGui-built)
// ============================================================
static void render_inspector() {
    const float w = 340.0f;
    ImGui::SetNextWindowPos(ImVec2(g_Engine.width - w, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w, (float)g_Engine.height), ImGuiCond_Always);

    ImGui::Begin("Inspector", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    Widget* sel = g_Engine.model.find(g_Engine.model.selectedId);
    if (!sel) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "No widget selected");
        ImGui::Separator();
        ImGui::TextWrapped("Click any widget on the canvas to select it, "
            "then edit its properties here.");
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f),
        "Selected: #%d", sel->id);
    ImGui::Separator();

    ImGui::Text("Type: %d", (int)sel->kind);
    ImGui::InputText("Label", sel->label, sizeof(sel->label));
    ImGui::InputText("Group", (char*)sel->groupName.c_str(),
                     sel->groupName.capacity() + 1);  // best effort

    ImGui::Separator();
    ImGui::DragFloat2("Position", &sel->x, 0.5f);
    ImGui::DragFloat("Width", &sel->w, 1.0f, 20.0f, 1000.0f);
    ImGui::DragFloat("Height (0=auto)", &sel->h, 0.5f, 0.0f, 500.0f);

    ImGui::Separator();
    switch (sel->kind) {
        case WidgetKind::Checkbox:
        case WidgetKind::Radio:
            ImGui::Checkbox("Checked", &sel->checked);
            break;
        case WidgetKind::SliderFloat:
        case WidgetKind::DragFloat:
            ImGui::SliderFloat("Value", &sel->floatValue, sel->minValue, sel->maxValue);
            ImGui::DragFloat("Min", &sel->minValue, 0.1f);
            ImGui::DragFloat("Max", &sel->maxValue, 0.1f);
            break;
        case WidgetKind::SliderInt:
            ImGui::SliderInt("Value", &sel->intValue, (int)sel->minValue, (int)sel->maxValue);
            break;
        case WidgetKind::InputText:
            ImGui::InputText("Value", sel->valueBuf, sizeof(sel->valueBuf));
            break;
        case WidgetKind::Combo:
            ImGui::SliderInt("Index", &sel->comboIndex,
                0, (int)sel->comboItems.size() - 1);
            break;
        case WidgetKind::ProgressBar:
            ImGui::SliderFloat("Progress", &sel->floatValue, 0.0f, 1.0f);
            break;
        default:
            break;
    }

    ImGui::Separator();
    if (ImGui::Button("Delete Widget", ImVec2(-1, 0))) {
        g_Engine.model.remove(sel->id);
    }

    ImGui::End();
}

// ============================================================
// Main render loop
// ============================================================
static void render_frame() {
    if (!g_Engine.eglReady || !g_Engine.imguiReady) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    // --- Top toolbar ---
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)g_Engine.width, 44.0f));
    ImGui::Begin("##toolbar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::SameLine();
    ImGui::Checkbox("Edit Mode", &g_Engine.model.editMode);
    ImGui::SameLine();
    ImGui::Checkbox("Show Inspector", &g_Engine.model.showInspector);
    ImGui::SameLine();
    if (ImGui::Button("Reset Demo")) g_Engine.model.initDemo();
    ImGui::SameLine();
    ImGui::Text("| %d widgets, %d selected",
        (int)g_Engine.model.widgets.size(), g_Engine.model.selectedId);
    ImGui::End();

    // --- Canvas window ---
    ImGui::SetNextWindowPos(ImVec2(0, 44));
    ImGui::SetNextWindowSize(ImVec2(
        (float)g_Engine.width - (g_Engine.model.showInspector ? 340.0f : 0.0f),
        (float)g_Engine.height - 44.0f));
    ImGui::Begin("##canvas", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Draw each group's background card
    ImDrawList* dl = ImGui::GetWindowDrawList();
    for (auto& grp : g_Engine.model.groups) {
        ImU32 bg = IM_COL32(30, 30, 30, 220);
        ImU32 border = IM_COL32(60, 60, 60, 255);
        dl->AddRectFilled(ImVec2(grp.x, grp.y),
                          ImVec2(grp.x + grp.w, grp.y + 340),
                          bg, 8.0f);
        dl->AddRect(ImVec2(grp.x, grp.y),
                    ImVec2(grp.x + grp.w, grp.y + 340),
                    border, 8.0f, 0, 1.5f);
        // Group title
        dl->AddText(ImVec2(grp.x + 12, grp.y + 8), IM_COL32(80, 160, 220, 255),
                    grp.name.c_str());
    }

    // Draw widgets
    for (auto& w : g_Engine.model.widgets) {
        render_widget(w);
    }

    ImGui::End();

    // --- Inspector ---
    if (g_Engine.model.showInspector) {
        render_inspector();
    }

    // --- Render to screen ---
    ImGui::Render();
    glViewport(0, 0, g_Engine.width, g_Engine.height);
    glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(g_Engine.display, g_Engine.surface);
}

// ============================================================
// Android callbacks
// ============================================================
static void handle_app_cmd(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != nullptr) {
                if (init_egl(app)) {
                    init_imgui(app);
                    g_Engine.animating = true;
                }
            }
            break;
        case APP_CMD_TERM_WINDOW:
            destroy_imgui();
            destroy_egl_surface_only();
            g_Engine.animating = false;
            break;
        case APP_CMD_GAINED_FOCUS:
            g_Engine.animating = true;
            break;
        case APP_CMD_LOST_FOCUS:
            g_Engine.animating = false;
            break;
        case APP_CMD_DESTROY:
            destroy_imgui();
            destroy_egl_all();
            break;
        default:
            break;
    }
}

static int32_t handle_input(android_app* app, AInputEvent* event) {
    if (!g_Engine.imguiReady) return 0;
    return ImGui_ImplAndroid_HandleInputEvent(event);
}

void android_main(android_app* app) {
    LOGI("android_main started");
    app->onAppCmd = handle_app_cmd;
    app->onInputEvent = handle_input;

    g_Engine.model.initDemo();

    while (true) {
        int events;
        android_poll_source* source;
        while (ALooper_pollAll(g_Engine.animating ? 0 : -1, nullptr,
                                &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
            if (app->destroyRequested != 0) {
                destroy_imgui();
                destroy_egl_all();
                LOGI("android_main exiting");
                return;
            }
        }
        if (g_Engine.animating && g_Engine.eglReady && g_Engine.imguiReady) {
            render_frame();
        }
    }
}
