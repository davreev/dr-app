if(TARGET dr::dr)
    return()
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE) # Show download progress

FetchContent_Declare(
    dr
    GIT_REPOSITORY git@github.com:davreev/dr.git
    GIT_TAG 464024462dc8b933b3f401824c774a169230c797
)

FetchContent_MakeAvailable(dr)
