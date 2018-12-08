
execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/mesh_files/basic.msh" "${CMAKE_CURRENT_BINARY_DIR}/basic.msh")
execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/mesh_files/decomposed.msh" "${CMAKE_CURRENT_BINARY_DIR}/decomposed.msh")

# generate tests
foreach(test ReaderTest)
    add_executable(${test} ${test}.cpp)
    target_link_libraries(${test} PRIVATE gmsh_reader Catch2::Catch2)
    add_test(${test} ${CMAKE_CURRENT_BUILD_DIR}/${test})
endforeach()
