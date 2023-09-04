add_executable(
    hello-triangle
    main.cpp
    scene.cpp
)

target_link_libraries(
    hello-triangle 
    PRIVATE
        dr::app
)

target_compile_options(
    hello-triangle
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)

target_compile_definitions(
    hello-triangle
    PRIVATE 
        # ...
)

target_link_options(
    hello-triangle
    PRIVATE
        # ...
)
