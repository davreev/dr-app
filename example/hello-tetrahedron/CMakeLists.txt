add_executable(
    hello-tetrahedron
    main.cpp
    scene.cpp
)

target_link_libraries(
    hello-tetrahedron 
    PRIVATE
        dr::app
)

target_compile_options(
    hello-tetrahedron
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)

target_compile_definitions(
    hello-tetrahedron
    PRIVATE 
        # ...
)

target_link_options(
    hello-tetrahedron
    PRIVATE
        # ...
)
