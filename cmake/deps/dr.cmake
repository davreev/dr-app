if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG da393a77cc8df4ecf1da5e92bc7a22e2bf3d9f9b
)

FetchContent_MakeAvailable(dr)
