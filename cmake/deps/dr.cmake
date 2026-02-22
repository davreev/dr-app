if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG 6857403726f4e4a9b63b7609b769aa03277484a9
)

FetchContent_MakeAvailable(dr)
