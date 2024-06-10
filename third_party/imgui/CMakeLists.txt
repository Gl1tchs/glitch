cmake_minimum_required(VERSION 3.25)

project(imgui)

set(IMGUI_SOURCES
    "imgui.h"
    "imgui_internal.h"
    "imgui.cpp"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_widgets.cpp"
    "imgui_tables.cpp"
    "misc/cpp/imgui_stdlib.h"
    "misc/cpp/imgui_stdlib.cpp"
)

add_library(imgui ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR})
