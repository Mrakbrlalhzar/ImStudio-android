cmake_minimum_required(VERSION 3.22)
project(imstudio_core)
set(CMAKE_CXX_STANDARD 17)

add_library(imgui STATIC
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC
    vendor/imgui
    vendor/imgui/misc/cpp
)

add_library(fmt INTERFACE)
target_include_directories(fmt INTERFACE vendor/fmt/include)
target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY=1)

set(IMS_SOURCES
    vendor/imstudio/ims_generator.cpp
    vendor/imstudio/ims_buffer.cpp
)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imstudio/ims_object.cpp)
    list(APPEND IMS_SOURCES vendor/imstudio/ims_object.cpp)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imstudio/ims_utils.cpp)
    list(APPEND IMS_SOURCES vendor/imstudio/ims_utils.cpp)
endif()

add_library(imstudio_core SHARED
    src/jni_bridge.cpp
    src/engine/buffer_accessor.cpp
    src/engine/code_generator.cpp
    ${IMS_SOURCES}
)
target_compile_definitions(imstudio_core PRIVATE FMT_HEADER_ONLY=1)
target_include_directories(imstudio_core PRIVATE
    include
    src
    vendor/imstudio
    vendor/imgui
    vendor/imgui/misc/cpp
)

find_library(log-lib log)
target_link_libraries(imstudio_core imgui fmt ${log-lib})
