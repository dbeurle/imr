
add_library(gmsh_reader gmsh/gmsh_reader.cpp element.cpp element_types.cpp)
target_link_libraries(gmsh_reader jsoncpp)
target_include_directories(gmsh_reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(abaqus_reader abaqus/abaqus_reader.cpp)
target_link_libraries(abaqus_reader jsoncpp)
target_include_directories(abaqus_reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(mesh_reader reader_factory.cpp)
target_link_libraries(mesh_reader abaqus_reader gmsh_reader)
