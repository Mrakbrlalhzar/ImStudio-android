#include "engine/buffer_accessor.h"
#include "ims_buffer.h"
#include "ims_object.h"
#include "imgui.h"

#include <android/log.h>
#include <mutex>
#include <new>

#define LOG_TAG "ImStudioCore"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace imstudio::engine {

static ImStudio::BufferWindow* g_BufferPtr = nullptr;
static std::mutex             g_Mutex;

static bool ensure_initialized_locked() {
    if (ImGui::GetCurrentContext() == nullptr) {
        LOGI("[ensure] Creating ImGui context...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        if (ImGui::GetCurrentContext() == nullptr) {
            LOGE("[ensure] FAILED: null ImGui context");
            return false;
        }
        LOGI("[ensure] ImGui context at %p", (void*)ImGui::GetCurrentContext());
    } else {
        LOGI("[ensure] ImGui context already at %p", (void*)ImGui::GetCurrentContext());
    }
    if (g_BufferPtr == nullptr) {
        LOGI("[ensure] Allocating BufferWindow...");
        g_BufferPtr = new (std::nothrow) ImStudio::BufferWindow();
        if (g_BufferPtr == nullptr) {
            LOGE("[ensure] FAILED: null BufferWindow");
            return false;
        }
        LOGI("[ensure] BufferWindow at %p", (void*)g_BufferPtr);
    }
    return true;
}

static ImStudio::BufferWindow& buf() {
    std::lock_guard<std::mutex> lock(g_Mutex);
    if (!ensure_initialized_locked())
        throw std::runtime_error("ImStudio engine not initialized");
    return *g_BufferPtr;
}

ImStudio::BufferWindow& get_buffer() { return buf(); }

void init() {
    LOGI("[init] entering");
    auto& b = buf();
    b.objects.clear();
    b.idgen = 0;
    b.selected_obj_id = -1;
    b.staticlayout = false;
    b.objects.reserve(2048);
    LOGI("[init] done, objects.size=%zu", b.objects.size());
}

void shutdown() {
    LOGI("[shutdown] entering");
    std::lock_guard<std::mutex> lock(g_Mutex);
    if (g_BufferPtr) { delete g_BufferPtr; g_BufferPtr = nullptr; }
    if (ImGui::GetCurrentContext() != nullptr) ImGui::DestroyContext();
    LOGI("[shutdown] done");
}

static ImStudio::Object* get_obj(int id) { return buf().getobj(id); }

int add_widget(const std::string& type) {
    LOGI("[add_widget] type=%s", type.c_str());
    auto& b = buf();
    b.create(type, false);
    int id = b.objects.empty() ? -1 : b.objects.back().id;
    LOGI("[add_widget] -> id=%d", id);
    return id;
}

bool delete_widget(int id) { if (auto* o = get_obj(id)) { o->del(); return true; } return false; }
int get_count() { return (int)buf().objects.size(); }
int get_id_at(int index) {
    auto& b = buf();
    if (index < 0 || index >= (int)b.objects.size()) return -1;
    return b.objects[index].id;
}
std::string get_type(int id)    { auto* o = get_obj(id); return o ? o->type : ""; }
std::string get_label(int id)   { auto* o = get_obj(id); return o ? o->label : ""; }
std::string get_value_s(int id) { auto* o = get_obj(id); return o ? o->value_s : ""; }
float get_x(int id)             { auto* o = get_obj(id); return o ? o->pos.x : 0.f; }
float get_y(int id)             { auto* o = get_obj(id); return o ? o->pos.y : 0.f; }
float get_size_x(int id)        { auto* o = get_obj(id); return o ? o->size.x : 0.f; }
float get_size_y(int id)        { auto* o = get_obj(id); return o ? o->size.y : 0.f; }
float get_width(int id)         { auto* o = get_obj(id); return o ? o->width : 0.f; }
bool set_label(int id, const std::string& v)   { auto* o = get_obj(id); if (!o) return false; o->label = v; return true; }
bool set_value_s(int id, const std::string& v) { auto* o = get_obj(id); if (!o) return false; o->value_s = v; return true; }
bool move(int id, float x, float y)            { auto* o = get_obj(id); if (!o) return false; o->pos.x = x; o->pos.y = y; return true; }
bool resize(int id, float sx, float sy)        { auto* o = get_obj(id); if (!o) return false; o->size.x = sx; o->size.y = sy; return true; }
bool set_width(int id, float w)                { auto* o = get_obj(id); if (!o) return false; o->width = w; return true; }

} // namespace imstudio::engine
