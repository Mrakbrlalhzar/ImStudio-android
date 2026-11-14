package com.raais.imstudio.model
enum class WidgetType(
    val displayName: String,
    val nativeName: String,
    val category: WidgetCategory
) {
    Button("دکمه", "button", WidgetCategory.Primitives),
    Radio("رادیو", "radio", WidgetCategory.Primitives),
    Checkbox("چک‌باکس", "checkbox", WidgetCategory.Primitives),
    Text("متن", "text", WidgetCategory.Primitives),
    Combo("کمبوباکس", "combo", WidgetCategory.Primitives),
    ListBox("لیست‌باکس", "listbox", WidgetCategory.Primitives),
    TextInput("ورودی متنی", "textinput", WidgetCategory.DataInputs),
    InputInt("ورودی صحیح", "inputint", WidgetCategory.DataInputs),
    InputFloat("ورودی اعشاری", "inputfloat", WidgetCategory.DataInputs),
    DragFloat("درگ اعشاری", "dragfloat", WidgetCategory.DataInputs),
    SliderFloat("اسلایدر اعشاری", "sliderfloat", WidgetCategory.DataInputs),
    Separator("جداکننده", "separator", WidgetCategory.Layout),
    Spacing("فاصله", "spacing", WidgetCategory.Layout)
}
