function(get_default_runtime_output_dir result_)
    if(CMAKE_CONFIGURATION_TYPES)
        set(result "${CMAKE_CURRENT_BINARY_DIR}/\$<CONFIG>")
    else()
        set(result "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    set(${result_} ${result} PARENT_SCOPE)
endfunction()


function(get_base_dir file result_)
    if(gen_dir AND file MATCHES "^${gen_dir}")
        set(result ${gen_dir})
    elseif(src_dir AND file MATCHES "^${src_dir}")
        set(result ${src_dir})
    elseif(shared_src_dir AND file MATCHES "^${shared_src_dir}")
        set(result ${shared_src_dir})
    else()
        message(FATAL_ERROR "Unexpected file location: ${file}")
    endif()
    set(${result_} ${result} PARENT_SCOPE)
endfunction()


function(copy_web_files)
    set(files_out)
    foreach(file_in ${web_files})
        get_base_dir(${file_in} base_dir)
        file(RELATIVE_PATH rel_path "${base_dir}/web" ${file_in})
        set(file_out "${runtime_output_dir}/${rel_path}")
        add_custom_command(
            OUTPUT
                ${file_out}
            DEPENDS 
                ${file_in}
            COMMAND 
                ${CMAKE_COMMAND} -E copy 
                ${file_in}
                ${file_out}
            COMMENT 
                "Copying ${file_in}"
        )
        list(APPEND files_out ${file_out})
    endforeach()

    add_custom_target(${app_name}-web DEPENDS ${files_out})
    add_dependencies(${app_name} ${app_name}-web)
endfunction()
