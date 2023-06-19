cmake_minimum_required(VERSION 3.14)
project(
    dr-app
    VERSION 0.1.0
    LANGUAGES C CXX
)

#
# Global config
#

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

list(
    PREPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_LIST_DIR}/cmake"
)

#
# Shim targets
#

add_library(
    sokol-shim STATIC
    src/shim/sokol.cpp
)

include(deps/imgui)
include(deps/sokol)

target_link_libraries(
    sokol-shim
    PUBLIC
        imgui::imgui
        sokol::sokol
)

# NOTE: sokol_app.h requires linking to platform-specific libraries (details in header)
if(EMSCRIPTEN)
    # ...
elseif(WIN32)
    # ...
elseif(APPLE)
    message(WARNING "Apple platforms are not supported")
    return()
elseif(UNIX)
    target_link_libraries(
        sokol-shim
        PRIVATE
            X11
            Xi
            Xcursor
            GL
            dl
            pthread
    )
endif()

#
# Library target
#

add_library(
    dr-app STATIC
    src/shim/imgui.cpp
    src/app.cpp
    src/app_config.c
    src/default_scene.cpp
    src/gfx.cpp
    src/task_queue.cpp
    src/thread_pool.cpp
    src/utils.cpp
)
add_library(dr::app ALIAS dr-app)

target_include_directories(
    dr-app
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
)

include(deps/dr)

target_link_libraries(
    dr-app
    PUBLIC
        dr::dr
        sokol-shim
)

target_compile_options(
    dr-app
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)

target_compile_definitions(
    dr-app
    PRIVATE 
        # ...
)

target_link_options(
    dr-app
    PRIVATE
        # ...
)

#
# Optional targets
#

option(DR_APP_EXAMPLE "Generate example targets" OFF)
if(DR_APP_EXAMPLE)
    add_subdirectory(example)
endif()
