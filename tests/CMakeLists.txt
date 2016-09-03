add_executable(exceptions exceptions.cpp)
target_link_libraries(exceptions LINK_PUBLIC GmshReader)
add_test(exceptions ${CMAKE_CURRENT_BUILD_DIR}/exceptions)
add_dependencies(exceptions catch)
