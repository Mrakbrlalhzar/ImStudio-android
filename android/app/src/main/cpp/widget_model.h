#pragma once
#include <string>
#include <vector>
#include <cstring>

// Widget types recognized by the model
enum class WidgetKind {
    Button, Checkbox, Radio, SliderFloat, SliderInt,
    DragFloat, InputText, Combo, Text, ProgressBar,
    Separator, SameLine, Spacing
};

struct Widget {
    int id = 0;
    WidgetKind kind = WidgetKind::Button;
    char label[128] = "Widget";     // for display / edit
    char valueBuf[256] = "";        // for InputText
    float x = 50.0f;
    float y = 50.0f;
    float w = 180.0f;
    float h = 0.0f;                 // 0 = auto
    bool checked = false;
    float floatValue = 0.5f;
    int intValue = 0;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    int comboIndex = 0;
    std::vector<std::string> comboItems;
    std::string groupName;          // For grouping in inspector
};

// A group is a logical section (like a window or tab).
struct WidgetGroup {
    std::string name;
    float x = 20.0f;
    float y = 20.0f;
    float w = 300.0f;
};

class WidgetModel {
public:
    std::vector<WidgetGroup> groups;
    std::vector<Widget> widgets;
    int selectedId = -1;
    int nextId = 1;
    bool editMode = true;   // true = drag to move; false = interact normally
    bool showInspector = true;

    void clear();
    void initDemo();        // populate with demo widgets
    Widget* find(int id);
    Widget* add(WidgetKind kind, const char* label, const char* group,
                float x, float y, float w = 180.0f);
    void remove(int id);
    int countInGroup(const std::string& group) const;
};
