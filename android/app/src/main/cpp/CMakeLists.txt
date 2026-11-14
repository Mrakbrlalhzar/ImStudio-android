cmake_minimum_required(VERSION 3.22)
project(imstudio_core)
set(CMAKE_CXX_STANDARD 17)

set(IMASSERT_HDR "${CMAKE_CURRENT_BINARY_DIR}/imassert_override.h")
file(WRITE "${IMASSERT_HDR}"
    "#pragma once\n#undef IM_ASSERT\n#define IM_ASSERT(_EXPR) ((void)0)\n")

add_library(imgui STATIC
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC vendor/imgui vendor/imgui/misc/cpp)
target_compile_options(imgui PUBLIC "-include" "${IMASSERT_HDR}")

add_library(fmt INTERFACE)
target_include_directories(fmt INTERFACE vendor/fmt/include)
target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY=1)

set(IMS_SOURCES
    vendor/imstudio/ims_generator.cpp
    vendor/imstudio/ims_buffer.cpp
    vendor/imstudio/ims_object.cpp
)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imstudio/ims_object_draw.cpp)
    list(APPEND IMS_SOURCES vendor/imstudio/ims_object_draw.cpp)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imstudio/ims_utils.cpp)
    list(APPEND IMS_SOURCES vendor/imstudio/ims_utils.cpp)
endif()

add_library(imstudio_core SHARED
    src/jni_bridge.cpp
    src/engine/buffer_accessor.cpp
    src/engine/code_generator.cpp
    src/engine/crash_handler.cpp
    ${IMS_SOURCES}
)
target_compile_definitions(imstudio_core PRIVATE FMT_HEADER_ONLY=1)
target_compile_options(imstudio_core PRIVATE "-include" "${IMASSERT_HDR}")
target_include_directories(imstudio_core PRIVATE
    include src
    vendor/imstudio vendor/imgui vendor/imgui/misc/cpp
    vendor/json)

find_library(log-lib log)
target_link_libraries(imstudio_core imgui fmt ${log-lib} dl)
