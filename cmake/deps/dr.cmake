if(TARGET dr::dr)
    return()
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE) # Show download progress

FetchContent_Declare(
    dr
    GIT_REPOSITORY git@github.com:davreev/dr.git
    GIT_TAG 8fb36a2e360c92fd64eb8d8bf09ffa6dbfcf0a00
)

FetchContent_MakeAvailable(dr)
