if(TARGET Tracy::TracyClient)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    tracy
    URL https://github.com/wolfpld/tracy/archive/refs/tags/v0.13.1.zip
)

option(DR_APP_TRACY "Enable Tracy profiler" OFF)

if(DR_APP_TRACY)
    if(EMSCRIPTEN)
        message(FATAL_ERROR "Tracy is not supported in Emscripten builds")
    endif()
    FetchContent_MakeAvailable(tracy)
    target_compile_options(TracyClient PRIVATE -w)
else()
    # NOTE(dr): When profiling is disabled, include Tracy as a header-only library so that macros
    # are still available
    FetchContent_GetProperties(tracy)
    if(NOT tracy_POPULATED)
        FetchContent_Populate(tracy)
    endif()
    add_library(TracyClient INTERFACE)
    add_library(Tracy::TracyClient ALIAS TracyClient)
    target_include_directories(
        TracyClient 
        SYSTEM # Suppress warnings from third party headers
        INTERFACE 
        "${tracy_SOURCE_DIR}/public"
    )
endif()
