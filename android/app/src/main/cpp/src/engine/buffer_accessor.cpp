#include "engine/buffer_accessor.h"
#include "ims_buffer.h"
#include "ims_object.h"
#include "imgui.h"
#include "json.hpp"
#include <android/log.h>
#include <mutex>
#include <new>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

#define LOG_TAG "ImStudioCore"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using nlohmann::json;

namespace imstudio::engine {

static ImStudio::BufferWindow* g_BufferPtr = nullptr;
static std::mutex g_Mutex;

static bool ensure_locked() {
    if (ImGui::GetCurrentContext() == nullptr) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        if (!ImGui::GetCurrentContext()) return false;
    }
    if (g_BufferPtr == nullptr) {
        g_BufferPtr = new (std::nothrow) ImStudio::BufferWindow();
        if (!g_BufferPtr) return false;
    }
    return true;
}
static ImStudio::BufferWindow& buf() {
    std::lock_guard<std::mutex> lock(g_Mutex);
    if (!ensure_locked()) throw std::runtime_error("engine not initialized");
    return *g_BufferPtr;
}
ImStudio::BufferWindow& get_buffer() { return buf(); }

void init() {
    auto& b = buf();
    b.objects.clear(); b.idgen = 0; b.selected_obj_id = -1;
    b.staticlayout = false; b.objects.reserve(2048);
}
void shutdown() {
    std::lock_guard<std::mutex> lock(g_Mutex);
    if (g_BufferPtr) { delete g_BufferPtr; g_BufferPtr = nullptr; }
    if (ImGui::GetCurrentContext()) ImGui::DestroyContext();
}
void clear_all() {
    auto& b = buf();
    b.objects.clear(); b.idgen = 0; b.selected_obj_id = -1;
}
static ImStudio::Object* get_obj(int id) { return buf().getobj(id); }

int add_widget(const std::string& type) {
    auto& b = buf(); b.create(type, false);
    return b.objects.empty() ? -1 : b.objects.back().id;
}
bool delete_widget(int id) { if (auto* o = get_obj(id)) { o->del(); return true; } return false; }
int get_count() { return (int)buf().objects.size(); }
int get_id_at(int i) { auto& b = buf(); if (i<0||i>=(int)b.objects.size()) return -1; return b.objects[i].id; }
std::string get_type(int id)    { auto* o=get_obj(id); return o?o->type:""; }
std::string get_label(int id)   { auto* o=get_obj(id); return o?o->label:""; }
std::string get_value_s(int id) { auto* o=get_obj(id); return o?o->value_s:""; }
float get_x(int id)      { auto* o=get_obj(id); return o?o->pos.x:0.f; }
float get_y(int id)      { auto* o=get_obj(id); return o?o->pos.y:0.f; }
float get_size_x(int id) { auto* o=get_obj(id); return o?o->size.x:0.f; }
float get_size_y(int id) { auto* o=get_obj(id); return o?o->size.y:0.f; }
float get_width(int id)  { auto* o=get_obj(id); return o?o->width:0.f; }
bool set_label(int id, const std::string& v)   { auto* o=get_obj(id); if(!o)return false; o->label=v; return true; }
bool set_value_s(int id, const std::string& v) { auto* o=get_obj(id); if(!o)return false; o->value_s=v; return true; }
bool move(int id, float x, float y)            { auto* o=get_obj(id); if(!o)return false; o->pos.x=x; o->pos.y=y; return true; }
bool resize(int id, float sx, float sy)        { auto* o=get_obj(id); if(!o)return false; o->size.x=sx; o->size.y=sy; return true; }
bool set_width(int id, float w)                { auto* o=get_obj(id); if(!o)return false; o->width=w; return true; }

// ---------- JSON dump/load ----------
static json make_obj_json(const ImStudio::Object& o) {
    return json{{"id",o.id},{"type",o.type},{"label",o.label},{"value_s",o.value_s},
        {"value_b",o.value_b},{"x",o.pos.x},{"y",o.pos.y},{"sizeX",o.size.x},
        {"sizeY",o.size.y},{"width",o.width},{"item_current",o.item_current}};
}
static void apply_json_to_obj(const json& j, ImStudio::Object& o) {
    if (j.contains("label") && j["label"].is_string()) o.label = j["label"].get<std::string>();
    if (j.contains("value_s") && j["value_s"].is_string()) o.value_s = j["value_s"].get<std::string>();
    if (j.contains("value_b") && j["value_b"].is_boolean()) o.value_b = j["value_b"].get<bool>();
    if (j.contains("pos") && j["pos"].is_object()) {
        o.pos.x = j["pos"].value("x", 0.f); o.pos.y = j["pos"].value("y", 0.f);
    } else {
        if (j.contains("x") && j["x"].is_number()) o.pos.x = j["x"].get<float>();
        if (j.contains("y") && j["y"].is_number()) o.pos.y = j["y"].get<float>();
    }
    if (j.contains("size") && j["size"].is_object()) {
        o.size.x = j["size"].value("x", 0.f); o.size.y = j["size"].value("y", 0.f);
    } else {
        if (j.contains("sizeX") && j["sizeX"].is_number()) o.size.x = j["sizeX"].get<float>();
        if (j.contains("sizeY") && j["sizeY"].is_number()) o.size.y = j["sizeY"].get<float>();
    }
    if (j.contains("width") && j["width"].is_number()) o.width = j["width"].get<float>();
    if (j.contains("item_current") && j["item_current"].is_number()) o.item_current = j["item_current"].get<int>();
}
bool load_from_text(const std::string& text) {
    json j;
    try { j = json::parse(text); } catch (...) { return false; }
    json arr;
    if (j.is_array()) arr = j;
    else if (j.is_object()) {
        if (j.contains("objects") && j["objects"].is_array()) arr = j["objects"];
        else if (j.contains("widgets") && j["widgets"].is_array()) arr = j["widgets"];
        else if (j.contains("items") && j["items"].is_array()) arr = j["items"];
        else return false;
    } else return false;
    try {
        clear_all(); auto& b = buf(); int maxId = 0, count = 0;
        for (const auto& item : arr) {
            if (!item.is_object()) continue;
            std::string type = item.value("type", "button");
            b.create(type, false);
            if (b.objects.empty()) continue;
            auto& o = b.objects.back();
            if (item.contains("id") && item["id"].is_number()) maxId = std::max(maxId, item["id"].get<int>());
            apply_json_to_obj(item, o); ++count;
        }
        if (maxId >= b.idgen) b.idgen = maxId + 1;
        return count > 0;
    } catch (...) { return false; }
}
std::string dump_to_json() {
    try {
        auto& b = buf(); json arr = json::array();
        for (auto& o : b.objects) arr.push_back(make_obj_json(o));
        return json{{"version",1},{"objects",arr}}.dump(2);
    } catch (...) { return "{\"version\":1,\"objects\":[]}"; }
}

// ============================================================
// Shared C++ parsing helpers
// ============================================================
static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
static std::string first_string_literal(const std::string& line) {
    auto q1 = line.find('"');
    if (q1 == std::string::npos) return "";
    auto q2 = line.find('"', q1 + 1);
    if (q2 == std::string::npos) return "";
    return line.substr(q1 + 1, q2 - q1 - 1);
}

struct ParsedWidget { std::string type; std::string label; float x = -1, y = -1; };

struct Pat { const char* needle; const char* type; };
static const std::vector<Pat>& widget_patterns() {
    static const std::vector<Pat> pats = {
        {"Button","button"}, {"SmallButton","button"}, {"InvisibleButton","button"},
        {"Checkbox","checkbox"}, {"RadioButton","radio"},
        {"SliderFloat","sliderfloat"}, {"SliderInt","sliderint"},
        {"DragFloat","dragfloat"}, {"DragInt","dragint"},
        {"InputText","textinput"}, {"InputTextWithHint","textinput"},
        {"InputInt","inputint"}, {"InputFloat","inputfloat"},
        {"Combo","combo"}, {"ListBox","listbox"},
        {"Text","text"}, {"TextUnformatted","text"}, {"TextWrapped","text"},
        {"TextDisabled","text"}, {"LabelText","text"},
        {"ProgressBar","progressbar"},
        {"Separator","separator"}, {"Spacing","spacing"},
        {"MenuItem","menuitem"}, {"TreeNode","treenode"},
        {"CollapsingHeader","treenode"},
        {"TabItemButton","button"}, {"TabItem","tab"}, {"BeginTabItem","tab"},
    };
    return pats;
}

static bool parse_line_for_widget(const std::string& line, ParsedWidget& out,
                                  float& lastSetX, float& lastSetY) {
    std::string trimmed = trim(line);
    if (trimmed.empty() || trimmed.rfind("//",0)==0) return false;

    if (trimmed.find("SetCursorPos") != std::string::npos) {
        auto vpos = trimmed.find("ImVec2");
        if (vpos != std::string::npos) {
            auto p1 = trimmed.find_first_of("-0123456789", vpos);
            if (p1 != std::string::npos) {
                char* end = nullptr;
                float x = std::strtof(trimmed.c_str()+p1, &end);
                if (end != trimmed.c_str()+p1) {
                    auto comma = trimmed.find(',', p1);
                    if (comma != std::string::npos) {
                        auto p2 = trimmed.find_first_of("-0123456789", comma);
                        if (p2 != std::string::npos) {
                            float y = std::strtof(trimmed.c_str()+p2, &end);
                            lastSetX = x; lastSetY = y;
                        }
                    }
                }
            }
        }
        return false;
    }

    for (auto& p : widget_patterns()) {
        std::string needle = std::string("ImGui::") + p.needle;
        auto pos = trimmed.find(needle);
        if (pos == std::string::npos) continue;
        size_t after = pos + needle.size();
        while (after < trimmed.size() && std::isspace((unsigned char)trimmed[after])) ++after;
        if (after >= trimmed.size() || trimmed[after] != '(') continue;

        out.type = p.type;
        out.label = first_string_literal(trimmed);
        out.x = lastSetX;
        out.y = lastSetY;
        lastSetX = -1; lastSetY = -1;
        return true;
    }
    return false;
}

static bool is_label_field_type(const std::string& t) {
    return t=="button"||t=="checkbox"||t=="radio"||t=="text"||
           t=="combo"||t=="listbox"||t=="menuitem"||t=="treenode"||t=="tab";
}

// ============================================================
// Single-file import (legacy)
// ============================================================
bool import_from_cpp(const std::string& code) {
    LOGI("[import_cpp] size=%zu", code.size());
    std::vector<std::string> lines; std::string cur;
    for (char c : code) {
        if (c == '\n') { lines.push_back(cur); cur.clear(); }
        else if (c != '\r') cur.push_back(c);
    }
    if (!cur.empty()) lines.push_back(cur);

    try {
        clear_all();
        auto& b = buf();
        int count = 0;
        float cursorX = 20.f, cursorY = 20.f;
        float lastSetX = -1, lastSetY = -1;

        for (auto& raw : lines) {
            ParsedWidget pw;
            if (!parse_line_for_widget(raw, pw, lastSetX, lastSetY)) continue;
            b.create(pw.type, false);
            if (b.objects.empty()) continue;
            auto& o = b.objects.back();
            if (!pw.label.empty()) {
                if (is_label_field_type(pw.type)) o.value_s = pw.label;
                else o.label = pw.label;
            }
            if (pw.x >= 0) { o.pos.x = pw.x; o.pos.y = pw.y; }
            else { o.pos.x = cursorX; o.pos.y = cursorY; cursorY += 44.f; }
            ++count;
        }
        return count > 0;
    } catch (const std::exception& e) {
        LOGE("[import_cpp] failed: %s", e.what());
        return false;
    }
}

// ============================================================
// ★ Multi-file import session state
// ============================================================
struct FileReport {
    std::string path;
    size_t size = 0;
    int lines_scanned = 0;
    int widgets_found = 0;
    int lines_non_imgui = 0;
    std::vector<std::string> recognized_widgets;   // unique type names
    std::vector<std::pair<int,std::string>> warnings;   // (line, message)
    std::vector<std::pair<int,std::string>> unrecognized_imgui;  // (line, fn name)
    std::string status;  // "ok", "no_widgets", "no_imgui", "parse_error"
};

struct ImportSessionState {
    bool active = false;
    int total_widgets = 0;
    std::vector<FileReport> file_reports;
    std::unordered_map<std::string, std::string> label_locations;  // "type:label" -> "path:line"
    std::vector<std::string> global_warnings;
};

static ImportSessionState g_ImportSession;

void import_begin() {
    LOGI("[import_begin]");
    g_ImportSession = ImportSessionState();
    g_ImportSession.active = true;
    clear_all();
}

void import_file(const std::string& path, const std::string& content) {
    LOGI("[import_file] %s (%zu bytes)", path.c_str(), content.size());
    FileReport rep;
    rep.path = path;
    rep.size = content.size();

    // Split lines
    std::vector<std::string> lines; std::string cur;
    for (char c : content) {
        if (c == '\n') { lines.push_back(cur); cur.clear(); }
        else if (c != '\r') cur.push_back(c);
    }
    if (!cur.empty()) lines.push_back(cur);
    rep.lines_scanned = (int)lines.size();

    try {
        auto& b = buf();
        float cursorX = 20.f, cursorY = 20.f;
        float lastSetX = -1, lastSetY = -1;
        std::unordered_set<std::string> recognized_set;

        for (size_t i = 0; i < lines.size(); ++i) {
            const std::string& raw = lines[i];
            std::string trimmed = trim(raw);
            if (trimmed.empty() || trimmed.rfind("//",0)==0) continue;

            ParsedWidget pw;
            if (parse_line_for_widget(raw, pw, lastSetX, lastSetY)) {
                b.create(pw.type, false);
                if (b.objects.empty()) {
                    rep.warnings.emplace_back((int)i+1, "create() failed");
                    continue;
                }
                auto& o = b.objects.back();
                if (!pw.label.empty()) {
                    if (is_label_field_type(pw.type)) o.value_s = pw.label;
                    else o.label = pw.label;

                    // Duplicate detection
                    std::string key = pw.type + ":" + pw.label;
                    auto it = g_ImportSession.label_locations.find(key);
                    if (it != g_ImportSession.label_locations.end()) {
                        std::string loc = path + ":" + std::to_string(i+1);
                        g_ImportSession.global_warnings.push_back(
                            "Duplicate '" + pw.label + "' (" + pw.type +
                            ") in " + loc + " — also at " + it->second);
                    } else {
                        g_ImportSession.label_locations[key] = path + ":" + std::to_string(i+1);
                    }
                }
                if (pw.x >= 0) { o.pos.x = pw.x; o.pos.y = pw.y; }
                else { o.pos.x = cursorX; o.pos.y = cursorY; cursorY += 44.f; }

                rep.widgets_found++;
                recognized_set.insert(pw.type);
                continue;
            }

            // Detect any ImGui:: call that isn't a widget
            auto igu = trimmed.find("ImGui::");
            if (igu != std::string::npos) {
                size_t s = igu + 7;
                size_t e = s;
                while (e < trimmed.size() &&
                       (std::isalnum((unsigned char)trimmed[e]) || trimmed[e] == '_')) ++e;
                std::string fn = trimmed.substr(s, e - s);
                if (!fn.empty()) rep.unrecognized_imgui.emplace_back((int)i+1, fn);
            } else {
                rep.lines_non_imgui++;
            }
        }

        for (auto& t : recognized_set) rep.recognized_widgets.push_back(t);

        if (rep.widgets_found == 0) {
            rep.status = rep.unrecognized_imgui.empty() ? "no_imgui" : "no_widgets";
        } else {
            rep.status = "ok";
        }

        g_ImportSession.total_widgets += rep.widgets_found;
        g_ImportSession.file_reports.push_back(std::move(rep));
    } catch (const std::exception& e) {
        LOGE("[import_file] exception: %s", e.what());
        rep.status = "parse_error";
        rep.warnings.emplace_back(0, std::string("exception: ") + e.what());
        g_ImportSession.file_reports.push_back(std::move(rep));
    }
}

std::string import_end() {
    LOGI("[import_end] files=%zu widgets=%d",
         g_ImportSession.file_reports.size(), g_ImportSession.total_widgets);
    json files = json::array();
    int with_widgets = 0, without_imgui = 0, total_warns = 0;

    for (auto& r : g_ImportSession.file_reports) {
        if (r.widgets_found > 0) with_widgets++;
        if (r.widgets_found == 0 && r.unrecognized_imgui.empty()) without_imgui++;
        total_warns += (int)r.warnings.size();

        json f = {
            {"path", r.path},
            {"size_bytes", (long long)r.size},
            {"lines_scanned", r.lines_scanned},
            {"widgets_found", r.widgets_found},
            {"lines_non_imgui", r.lines_non_imgui},
            {"status", r.status},
            {"recognized_widgets", r.recognized_widgets}
        };

        json warns = json::array();
        for (auto& w : r.warnings) {
            warns.push_back({{"line", w.first}, {"message", w.second}});
        }
        f["warnings"] = warns;

        json uis = json::array();
        for (auto& u : r.unrecognized_imgui) {
            uis.push_back({{"line", u.first}, {"text", "ImGui::" + u.second + "(...)"}});
        }
        f["unrecognized_imgui"] = uis;

        files.push_back(f);
    }

    json root = {
        {"session", {
            {"total_files", (int)g_ImportSession.file_reports.size()},
            {"files_with_widgets", with_widgets},
            {"files_without_imgui", without_imgui},
            {"total_widgets", g_ImportSession.total_widgets},
            {"total_warnings", total_warns},
            {"total_global_warnings", (int)g_ImportSession.global_warnings.size()}
        }},
        {"files", files},
        {"global_warnings", g_ImportSession.global_warnings}
    };

    g_ImportSession.active = false;
    return root.dump(2);
}

} // namespace imstudio::engine
