if(TARGET imgui::imgui)
    return()
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE) # Show download progress

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG 9aae45eb4a05a5a1f96be1ef37eb503a12ceb889 # Version 1.88
    GIT_PROGRESS TRUE
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
