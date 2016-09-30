
# copy mesh file to binary dir
configure_file(${CMAKE_SOURCE_DIR}/mesh_files/feti_beam.msh ${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(first_example first_example.cpp)
TARGET_LINK_LIBRARIES(first_example LINK_PUBLIC GmshReader)

# Basic single mesh example
configure_file(${CMAKE_SOURCE_DIR}/mesh_files/basic.msh ${CMAKE_CURRENT_BINARY_DIR})
add_executable(basic_mesh basic_mesh.cpp)
target_link_libraries(BasicMesh LINK_PUBLIC GmshReader)

# Decomposed mesh example
configure_file(${CMAKE_SOURCE_DIR}/mesh_files/decomposed.msh ${CMAKE_CURRENT_BINARY_DIR})
add_executable(decomposed_mesh decomposed_mesh.cpp)
target_link_libraries(DecomposedMesh LINK_PUBLIC GmshReader)
