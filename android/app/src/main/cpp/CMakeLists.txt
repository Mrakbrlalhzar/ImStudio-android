cmake_minimum_required(VERSION 3.22)
project(imguieditor)
set(CMAKE_CXX_STANDARD 17)

# --- native_app_glue from NDK ---
set(GLUE_DIR "")
foreach(c "${ANDROID_NDK}/sources/android/native_app_glue"
          "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")
    if(EXISTS "${c}/android_native_app_glue.c") set(GLUE_DIR "${c}"); break(); endif()
endforeach()
if(NOT GLUE_DIR)
    message(FATAL_ERROR "native_app_glue not found in NDK")
endif()

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")

# --- ImGui core + backends (no GLFW!) ---
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_android.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp
)

# --- Our app ---
add_library(imguieditor SHARED
    android_main.cpp
    widget_model.cpp
    ${GLUE_DIR}/android_native_app_glue.c
)
target_include_directories(imguieditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLUE_DIR}
)

find_library(log-lib log)
target_link_libraries(imguieditor imgui android EGL GLESv3 ${log-lib})
target_link_options(imguieditor PRIVATE "-u" "ANativeActivity_onCreate")
