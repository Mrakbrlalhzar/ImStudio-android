cmake_minimum_required(VERSION 3.22)
project(imguieditor)
set(CMAKE_CXX_STANDARD 17)

# 导入 Dear ImGui Bundle
add_subdirectory(/tmp/imgui_bundle ${CMAKE_CURRENT_BINARY_DIR}/imgui_bundle)

# 使用 Bundle 的宏构建应用 (自动链接 ImGui, ImPlot, node-editor 等)
imgui_bundle_add_app(imguieditor
    android_main.cpp
    widget_model.cpp
)

# 定义 Docking 宏 (确保 ImGui 内部启用 Docking 相关功能)
target_compile_definitions(imguieditor PRIVATE IMGUI_DOCKING=1)

# 链接 Android 系统库
find_library(log-lib log)
target_link_libraries(imguieditor
    android
    EGL
    GLESv3
    ${log-lib}
)

# 强制保留 ANativeActivity_onCreate
target_link_options(imguieditor PRIVATE "-u" "ANativeActivity_onCreate")
