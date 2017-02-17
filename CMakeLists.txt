cmake_minimum_required (VERSION 2.8)

project(huffmanZipper)

set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH ..)
set(CMAKE_CXX_FLAGS -g)

aux_source_directory(./src DIR_SRCS)
add_executable(mainZipper src/main.cpp ${DIR_SRCS})

target_link_libraries(mainZipper glog)
target_link_libraries(mainZipper pthread)
