if(TARGET sokol::sokol)
    return()
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE) # Show download progress

FetchContent_Declare(
    sokol
    GIT_REPOSITORY git@github.com:davreev/sokol.git
    GIT_TAG a53aac381384de7c115958167409ca2024c47a79
    GIT_PROGRESS TRUE
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