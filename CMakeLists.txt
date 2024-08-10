cmake_minimum_required(VERSION 3.14)
project(
    dr-app
    VERSION 0.2.0
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

# Show FetchContent download progress
set(FETCHCONTENT_QUIET FALSE)

include(deps/dr)
include(deps/imgui)
include(deps/sokol)

#
# Util target
#

add_library(
    dr-app-util STATIC
    src/camera.cpp
    src/file_utils.cpp
    src/gfx_resource.cpp
    src/gfx_utils.cpp
    src/task_queue.cpp
    src/thread_pool.cpp
)
add_library(dr::app-util ALIAS dr-app-util)

target_include_directories(
    dr-app-util
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
)

target_link_libraries(
    dr-app-util
    PUBLIC
        dr::dr
        sokol::sokol
)

target_compile_options(
    dr-app-util
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)

if(EMSCRIPTEN)
    # Enable use of std::thread
    target_compile_options(dr-app-util PRIVATE -pthread)
    target_link_options(dr-app-util PRIVATE -pthread)
endif()

#
# App target
#

if(APPLE AND NOT EMSCRIPTEN)
    # NOTE: sokol_app.h implementation must compiled as Objective-C on macOS
    add_library(
        dr-app-sokol STATIC
        "src/shim/sokol.mm"
    )
else()
    add_library(
        dr-app-sokol STATIC
        "src/shim/sokol.cpp"
    )
endif()

target_link_libraries(
    dr-app-sokol
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
    target_link_libraries(
        dr-app-sokol
        PRIVATE
            "-framework Cocoa"
            "-framework QuartzCore"
            "-framework OpenGL"
    )
elseif(UNIX)
    target_link_libraries(
        dr-app-sokol
        PRIVATE
            X11
            Xi
            Xcursor
            GL
            dl
            pthread
    )
endif()

add_library(
    dr-app STATIC
    src/app.cpp
    src/app.c
    src/event_handlers.cpp
    src/shim/imgui.cpp
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
        dr-app-sokol
        dr-app-util
)

target_compile_options(
    dr-app
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)

#
# Optional targets
#

option(DR_APP_EXAMPLE "Generate example targets" OFF)
if(DR_APP_EXAMPLE)
    add_subdirectory(example)
endif()

option(DR_APP_TEST "Generate test target" OFF)
if(DR_APP_TEST)
    enable_testing()
    add_subdirectory(test)
endif()
