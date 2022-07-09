cmake_minimum_required(VERSION 3.22)
project(RTree)

set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
add_executable(RTree main.cpp RTree.cpp RTree.h HilbertRtree.h HilbertRtree.cpp lib/utils.cpp lib/utils.h lib/testing.cpp lib/testing.h lib/rapidcsv.h)
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries( RTree ${OpenCV_LIBS} )