if(TARGET dr::dr)
    return()
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE) # Show download progress

FetchContent_Declare(
    dr
    GIT_REPOSITORY git@github.com:davreev/dr.git
    GIT_TAG ecfe8495f7bab00e1324cce34fb49c8f34fbe304
)

FetchContent_MakeAvailable(dr)
