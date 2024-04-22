if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY git@github.com:davreev/dr.git
    GIT_TAG 26321faa88f39c56f90d1ff1cbe99bc718b126ae # 0.1.0
)

FetchContent_MakeAvailable(dr)
