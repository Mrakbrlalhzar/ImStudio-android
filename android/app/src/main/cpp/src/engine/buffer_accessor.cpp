#include "engine/buffer_accessor.h"
#include "vendor/imstudio/ims_buffer.h"
#include "vendor/imstudio/ims_object.h"

namespace imstudio::engine {

static ImStudio::BufferWindow g_Buffer;

static ImStudio::Object* get_obj(int id) {
    return g_Buffer.getobj(id);
}

void init() {
    g_Buffer.objects.clear();
    g_Buffer.idgen = 0;
    g_Buffer.selected_obj_id = -1;
    g_Buffer.staticlayout = false;
    g_Buffer.objects.reserve(2048);
}

int add_widget(const std::string& type) {
    g_Buffer.create(type, false);
    return g_Buffer.objects.empty() ? -1 : g_Buffer.objects.back().id;
}

bool delete_widget(int id) {
    if (auto* o = get_obj(id)) { o->del(); return true; }
    return false;
}

int get_count() { return (int)g_Buffer.objects.size(); }

int get_id_at(int index) {
    if (index < 0 || index >= (int)g_Buffer.objects.size()) return -1;
    return g_Buffer.objects[index].id;
}

std::string get_type(int id) {
    auto* o = get_obj(id);
    return o ? o->type : "";
}
std::string get_label(int id) {
    auto* o = get_obj(id);
    return o ? o->label : "";
}
std::string get_value_s(int id) {
    auto* o = get_obj(id);
    return o ? o->value_s : "";
}
float get_x(int id)       { auto* o = get_obj(id); return o ? o->pos.x : 0.f; }
float get_y(int id)       { auto* o = get_obj(id); return o ? o->pos.y : 0.f; }
float get_size_x(int id)  { auto* o = get_obj(id); return o ? o->size.x : 0.f; }
float get_size_y(int id)  { auto* o = get_obj(id); return o ? o->size.y : 0.f; }
float get_width(int id)   { auto* o = get_obj(id); return o ? o->width : 0.f; }

bool set_label(int id, const std::string& v) {
    auto* o = get_obj(id); if (!o) return false; o->label = v; return true;
}
bool set_value_s(int id, const std::string& v) {
    auto* o = get_obj(id); if (!o) return false; o->value_s = v; return true;
}
bool move(int id, float x, float y) {
    auto* o = get_obj(id); if (!o) return false;
    o->pos.x = x; o->pos.y = y; return true;
}
bool resize(int id, float sx, float sy) {
    auto* o = get_obj(id); if (!o) return false;
    o->size.x = sx; o->size.y = sy; return true;
}
bool set_width(int id, float w) {
    auto* o = get_obj(id); if (!o) return false; o->width = w; return true;
}

} // namespace imstudio::engine
