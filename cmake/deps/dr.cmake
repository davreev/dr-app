if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

# NOTE(dr): Pinned to specific revision until 0.4.0
#[[
FetchContent_Declare(
    dr
    URL https://github.com/davreev/dr/archive/refs/tags/0.3.0.zip
)
]]
FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG 82b866b16ce72cc6463cd5f1517b0ca0e902ab34
)

FetchContent_MakeAvailable(dr)
