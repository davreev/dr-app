if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG d1390f2408b16b54ec580cbeba99e746111e0ee0
)

# Disable OpenMP for Emscripten builds
if(EMSCRIPTEN)
    set(DR_OPENMP OFF)
endif()

FetchContent_MakeAvailable(dr)
