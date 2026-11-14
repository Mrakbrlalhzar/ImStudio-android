cmake_minimum_required(VERSION 3.22)
project(imgui_editor)
set(CMAKE_CXX_STANDARD 17)

# ---- Locate native_app_glue ----
set(NATIVE_APP_GLUE_DIR "")
foreach(candidate
        "${ANDROID_NDK}/sources/android/native_app_glue"
        "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue"
        "${ANDROID_NDK_ROOT}/sources/android/native_app_glue")
    if(EXISTS "${candidate}/android_native_app_glue.c")
        set(NATIVE_APP_GLUE_DIR "${candidate}")
        message(STATUS "native_app_glue at ${candidate}")
        break()
    endif()
endforeach()
if(NOT NATIVE_APP_GLUE_DIR)
    message(FATAL_ERROR "native_app_glue not found in NDK")
endif()

# ---- ImGui core + backends ----
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_android.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ---- Our editor app ----
add_library(imgui_editor SHARED
    android_main.cpp
    widget_model.cpp
    ${NATIVE_APP_GLUE_DIR}/android_native_app_glue.c
)
target_include_directories(imgui_editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NATIVE_APP_GLUE_DIR}
)

find_library(log-lib log)
target_link_libraries(imgui_editor
    imgui
    android
    EGL
    GLESv3
    ${log-lib}
)

# Force-keep ANativeActivity_onCreate
target_link_options(imgui_editor PRIVATE "-u" "ANativeActivity_onCreate")
