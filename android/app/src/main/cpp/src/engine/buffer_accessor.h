#pragma once
#include <string>

namespace imstudio::engine {
    void init();
    int  add_widget(const std::string& type);
    bool delete_widget(int id);
    int  get_count();
    int  get_id_at(int index);
    std::string get_type(int id);
    std::string get_label(int id);
    std::string get_value_s(int id);
    float get_x(int id);
    float get_y(int id);
    float get_size_x(int id);
    float get_size_y(int id);
    float get_width(int id);
    bool set_label(int id, const std::string& v);
    bool set_value_s(int id, const std::string& v);
    bool move(int id, float x, float y);
    bool resize(int id, float sx, float sy);
    bool set_width(int id, float w);
}
