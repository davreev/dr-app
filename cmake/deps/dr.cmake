if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY git@github.com:davreev/dr.git
    GIT_TAG a616960669c6d4c67ee5771435ed41c2b35f3dd5 # 0.1.0
)

FetchContent_MakeAvailable(dr)
