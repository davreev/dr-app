if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG f9c2b99f39f9a826ee0e98a4a032c162d19eea88
)

# Disable OpenMP for Emscripten builds
if(EMSCRIPTEN)
    set(DR_OPENMP OFF)
endif()

FetchContent_MakeAvailable(dr)
