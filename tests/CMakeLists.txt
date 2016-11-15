
execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/mesh_files/basic.msh" "${CMAKE_CURRENT_BINARY_DIR}/basic.msh")

# generate tests
foreach(test
        exceptions
    )

    add_executable(${test} ${test}.cpp)
    target_link_libraries(${test} LINK_PUBLIC GmshReader)
    add_test(${test} ${CMAKE_CURRENT_BUILD_DIR}/${test})
    add_dependencies(${test} catch)
endforeach()
