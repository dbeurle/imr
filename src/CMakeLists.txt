
ADD_LIBRARY(GmshReader GmshReader.cpp ElementData.cpp)
target_link_libraries(GmshReader jsoncpp)
TARGET_INCLUDE_DIRECTORIES(GmshReader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
