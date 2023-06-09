find_package(
        Python3
        REQUIRED
        COMPONENTS Interpreter
)



set(PYTHON_SCRIPT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

function(RunScript script_file_dir args)
    set (py_cmd ${PYTHON_SCRIPT_DIR}/${script_file_dir} ${args})
    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${py_cmd}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE py_result
    )
    message(STATUS "Python3 [${Python3_EXECUTABLE}] (${script_file_dir}) result: ${py_result}")
endfunction()


function(RunModule module_name args)
    set (py_cmd -m ${module_name} ${args})
    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${py_cmd}
    )
    message(STATUS "Python3.10 -m (${module_name}) result: ${py_result}")
endfunction()

