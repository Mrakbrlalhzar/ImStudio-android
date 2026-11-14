#include "widget_model.h"
#include <cstring>
#include <algorithm>

void WidgetModel::clear() {
    groups.clear(); widgets.clear(); selectedId = -1; nextId = 1;
}

Widget* WidgetModel::find(int id) {
    for (auto& w : widgets) if (w.id == id) return &w;
    return nullptr;
}

Widget* WidgetModel::add(WidgetKind kind, const char* label, const char* group,
                         float x, float y, float width) {
    Widget w;
    w.id = nextId++;
    w.kind = kind;
    std::strncpy(w.label, label ? label : "", sizeof(w.label) - 1);
    w.label[sizeof(w.label) - 1] = 0;
    w.x = x; w.y = y; w.w = width; w.h = 0.0f;
    w.groupName = group ? group : "";
    widgets.push_back(w);
    return &widgets.back();
}

void WidgetModel::remove(int id) {
    widgets.erase(std::remove_if(widgets.begin(), widgets.end(),
        [id](const Widget& w) { return w.id == id; }), widgets.end());
    if (selectedId == id) selectedId = -1;
}

void WidgetModel::initDemo() {
    clear();
    groups.push_back({"Aimbot", 30, 30, 320});
    groups.push_back({"ESP",     30, 30, 320});
    groups.push_back({"Misc",    30, 30, 320});
    groups.push_back({"About",   400, 30, 320});

    float y = 80.0f;
    auto* w = add(WidgetKind::Checkbox, "Enable Aimbot", "Aimbot", 50, y); w->checked = true;
    w = add(WidgetKind::SliderFloat, "FOV", "Aimbot", 50, y += 40);
    w->minValue = 0.0f; w->maxValue = 180.0f; w->floatValue = 45.0f;
    w = add(WidgetKind::SliderFloat, "Smoothing", "Aimbot", 50, y += 40);
    w->minValue = 0.0f; w->maxValue = 100.0f; w->floatValue = 30.0f;
    w = add(WidgetKind::Combo, "Target Bone", "Aimbot", 50, y += 40);
    w->comboItems = {"Head", "Chest", "Pelvis", "Legs"};
    add(WidgetKind::Button, "Apply Aim Settings", "Aimbot", 50, y += 40, 240);

    y = 80.0f;
    w = add(WidgetKind::Checkbox, "Enable ESP", "ESP", 50, y); w->checked = true;
    w = add(WidgetKind::Checkbox, "Box ESP", "ESP", 50, y += 36); w->checked = true;
    add(WidgetKind::Checkbox, "Health Bar", "ESP", 50, y += 36);
    w = add(WidgetKind::SliderFloat, "Thickness", "ESP", 50, y += 40);
    w->minValue = 0.5f; w->maxValue = 5.0f; w->floatValue = 1.5f;
    add(WidgetKind::Button, "Reset ESP", "ESP", 50, y += 40, 200);

    y = 80.0f;
    w = add(WidgetKind::Checkbox, "Anti-Flash", "Misc", 50, y); w->checked = true;
    w = add(WidgetKind::SliderFloat, "FOV Changer", "Misc", 50, y += 40);
    w->minValue = 60.0f; w->maxValue = 120.0f; w->floatValue = 90.0f;
    w = add(WidgetKind::InputText, "Player Name", "Misc", 50, y += 40);
    std::strncpy(w->valueBuf, "Player", sizeof(w->valueBuf) - 1);
    add(WidgetKind::Button, "Save Config", "Misc", 50, y += 50, 200);

    y = 80.0f;
    add(WidgetKind::Text, "ImGui Visual Engine", "About", 420, y);
    add(WidgetKind::Text, "Full Bundle + Dual Mode", "About", 420, y += 28);
    add(WidgetKind::Separator, "", "About", 420, y += 30);
    add(WidgetKind::Text, "Click Test Mode to enter test screen", "About", 420, y += 20);
    add(WidgetKind::Text, "Touch points are visualized there", "About", 420, y += 20);
    add(WidgetKind::Button, "Reset Demo", "About", 420, y += 40, 200);
}
