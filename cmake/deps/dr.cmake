if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG 189ab2427ae623f1954b6607c2bbb536c4a5d454
)

FetchContent_MakeAvailable(dr)
