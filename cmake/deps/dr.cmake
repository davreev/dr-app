if(TARGET dr::dr)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    dr
    GIT_REPOSITORY https://github.com/davreev/dr.git
    GIT_TAG 9676cc16de0ed5bdc698cca82e5054bf4b0c8109
)

FetchContent_MakeAvailable(dr)
