if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG fb06406caa5885d26c88176a7421003a2e5f7932
)

FetchContent_MakeAvailable(dr)
