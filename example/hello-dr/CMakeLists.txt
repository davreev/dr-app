add_executable(
    hello-dr
    main.cpp
    scene.cpp
)

target_link_libraries(
    hello-dr 
    PRIVATE
        dr::app
)

target_compile_options(
    hello-dr
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)

target_compile_definitions(
    hello-dr
    PRIVATE 
        # ...
)

target_link_options(
    hello-dr
    PRIVATE
        # ...
)
