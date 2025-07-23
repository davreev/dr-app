if(TARGET implot::implot)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    implot
    URL https://github.com/epezent/implot/archive/refs/tags/v0.16.zip
)

FetchContent_GetProperties(implot)
if(NOT ${implot_POPULATED})
    FetchContent_Populate(implot)
endif()

add_library(
    implot STATIC
    "${implot_SOURCE_DIR}/implot.cpp"
    "${implot_SOURCE_DIR}/implot_demo.cpp"
    "${implot_SOURCE_DIR}/implot_items.cpp"
)
add_library(implot::implot ALIAS implot)

target_include_directories(
    implot 
    SYSTEM # Suppresses warnings from third party headers
    PUBLIC
        "${implot_SOURCE_DIR}"
)

include(deps/imgui)

target_link_libraries(
    implot
    PRIVATE
        imgui::imgui
)
