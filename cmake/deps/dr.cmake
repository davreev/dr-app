if(TARGET dr::dr)
    return()
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE) # Show download progress

FetchContent_Declare(
    dr
    GIT_REPOSITORY git@github.com:davreev/dr.git
    GIT_TAG dd097182b16be763dee47d69d25539bf673a1f64
)

FetchContent_MakeAvailable(dr)
