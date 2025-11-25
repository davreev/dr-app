if(TARGET implot3d::implot3d)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    implot3d
    URL https://github.com/brenocq/implot3d/archive/refs/tags/v0.2.zip
)

FetchContent_GetProperties(implot3d)
if(NOT ${implot3d_POPULATED})
    FetchContent_Populate(implot3d)
endif()

add_library(
    implot3d STATIC
    "${implot3d_SOURCE_DIR}/implot3d.cpp"
    "${implot3d_SOURCE_DIR}/implot3d_demo.cpp"
    "${implot3d_SOURCE_DIR}/implot3d_items.cpp"
    "${implot3d_SOURCE_DIR}/implot3d_meshes.cpp"
)
add_library(implot3d::implot3d ALIAS implot3d)

target_include_directories(
    implot3d 
    SYSTEM # Suppress warnings from third party headers
    PUBLIC
        "${implot3d_SOURCE_DIR}"
)

include(deps/imgui)

target_link_libraries(
    implot3d
    PRIVATE
        imgui::imgui
)

# Disable warnings from third party source
target_compile_options(implot3d PRIVATE -w)
