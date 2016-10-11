
# copy mesh file to binary dir
configure_file(${CMAKE_SOURCE_DIR}/mesh_files/feti_beam_fine.msh ${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(first_example first_example.cpp)
TARGET_LINK_LIBRARIES(first_example LINK_PUBLIC GmshReader)

# Basic single mesh example
configure_file(${CMAKE_SOURCE_DIR}/mesh_files/basic.msh ${CMAKE_CURRENT_BINARY_DIR})
add_executable(BasicMesh BasicMesh.cpp)
target_link_libraries(BasicMesh LINK_PUBLIC GmshReader)

# Decomposed mesh example
configure_file(${CMAKE_SOURCE_DIR}/mesh_files/decomposed.msh ${CMAKE_CURRENT_BINARY_DIR})
add_executable(DecomposedMesh DecomposedMesh.cpp)
target_link_libraries(DecomposedMesh LINK_PUBLIC GmshReader)

# four point bending example
execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/mesh_files/fourPointBending.msh" "${CMAKE_CURRENT_BINARY_DIR}/fourPointBending.msh")
set(file fourPointBending)
add_executable(${file} ${file}.cpp)
target_link_libraries(${file} LINK_PUBLIC GmshReader)

# four point bending example
execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/mesh_files/2d_miehe_symmetric_three_point_bending.msh" "${CMAKE_CURRENT_BINARY_DIR}/threePointBending.msh")
set(file threePointBending)
add_executable(${file} ${file}.cpp)
target_link_libraries(${file} LINK_PUBLIC GmshReader)
