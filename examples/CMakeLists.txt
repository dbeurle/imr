
# copy mesh file to binary dir
configure_file(${CMAKE_SOURCE_DIR}/mesh_files/feti_beam.msh ${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(first_example first_example.cpp)
TARGET_LINK_LIBRARIES(first_example LINK_PUBLIC GmshReader)

configure_file(${CMAKE_SOURCE_DIR}/mesh_files/basic.msh ${CMAKE_CURRENT_BINARY_DIR})
add_executable(MurgeMesh MurgeMesh.cpp)
target_link_libraries(MurgeMesh LINK_PUBLIC GmshReader)
