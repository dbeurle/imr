
add_library(reader mesh_reader.cpp element.cpp)
target_link_libraries(reader jsoncpp)
target_include_directories(reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
