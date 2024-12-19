if(TARGET imgui::imgui)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.4.zip
)

FetchContent_GetProperties(imgui)
if(NOT ${imgui_POPULATED})
    FetchContent_Populate(imgui)
endif()

add_library(
    imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
)
add_library(imgui::imgui ALIAS imgui)

target_include_directories(
    imgui 
    PUBLIC
        "${imgui_SOURCE_DIR}"
)
