add_executable(
    dr-app-test 
    main.cpp
    task_queue_tests.cpp
)

include(deps/utest)

target_link_libraries(
    dr-app-test 
    PRIVATE
        dr-app
        utest::utest
)

target_compile_options(
    dr-app-test
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)

target_compile_definitions(
    dr-app-test
    PRIVATE 
        # ...
)

target_link_options(
    dr-app-test
    PRIVATE
        # ...
)

add_test(
    NAME dr-app-test
    COMMAND dr-app-test
)
