set(app_name hello-tetra)

add_executable(
    ${app_name}
    main.cpp
    scene.cpp
)

target_link_libraries(
    ${app_name} 
    PRIVATE
        dr::app
)

target_compile_options(
    ${app_name}
    PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
)

if(EMSCRIPTEN)
    # Emscripten compiler options
    target_link_options(
        ${app_name}
        PRIVATE
            "-sUSE_WEBGL2=1"
            "$<$<CONFIG:Debug>:-sASSERTIONS=2>"
            "$<$<CONFIG:Debug>:-gsource-map>"
    )
endif()

#
# Post-build commands
#

set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")
get_default_runtime_output_dir(runtime_output_dir)

if(EMSCRIPTEN)
    set(
        web_files
        "${src_dir}/web/index.html"
        "${shared_src_dir}/web/coi-serviceworker.js"
        # ...
    )
    copy_web_files()
endif()
