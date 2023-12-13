add_executable(
    hello-tetra
    main.cpp
    scene.cpp
)

target_link_libraries(
    hello-tetra 
    PRIVATE
        dr::app
)

target_compile_options(
    hello-tetra
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)
