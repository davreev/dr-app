if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG 014a7112b825aa13650c28ae0851b8ef152c3f7a
)

# Disable OpenMP for Emscripten builds
if(EMSCRIPTEN)
    set(DR_OPENMP OFF)
endif()

FetchContent_MakeAvailable(dr)

# DEBUG(dr): Use local dr
# add_subdirectory("/home/dave/dev/dr" "${CMAKE_BINARY_DIR}/_deps/dr-local-build" )