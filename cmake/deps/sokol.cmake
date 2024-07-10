if(TARGET sokol::sokol)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    sokol
    URL https://github.com/davreev/sokol/archive/refs/tags/dr-app-0.2.0.zip
)

FetchContent_GetProperties(sokol)
if(NOT ${sokol_POPULATED})
    FetchContent_Populate(sokol)
endif()

add_library(sokol INTERFACE)
add_library(sokol::sokol ALIAS sokol)

target_include_directories(
    sokol 
    SYSTEM # Suppresses warnings
    INTERFACE 
        "${sokol_SOURCE_DIR}"
        "${sokol_SOURCE_DIR}/util"
)