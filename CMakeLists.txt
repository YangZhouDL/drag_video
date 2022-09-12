cmake_minimum_required(VERSION 2.8.3)
project(drag_video)

find_package(catkin_simple REQUIRED)

#uncomment next line to use OpenCV library
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#uncomment the next 2 lines to use the point-cloud library
#find_package(PCL 1.7 REQUIRED)
#include_directories(${PCL_INCLUDE_DIRS})


#uncomment the following 4 lines to use the Eigen library
#find_package(cmake_modules REQUIRED)
#find_package(Eigen3 REQUIRED)
#include_directories(${EIGEN3_INCLUDE_DIR})
#add_definitions(${EIGEN_DEFINITIONS})

#use FFMPEG library
set(FFMPEG_LIBS_DIR /usr/lib/x86_64-linux-gnu)
set(FFMPEG_HEADERS_DIR /usr/include/x86_64-linux-gnu)
include_directories(${FFMPEG_HEADERS_DIR})
link_directories(${FFMPEG_LIBS_DIR})
set(FFMPEG_LIBS libavcodec.so libavformat.so libswscale.so libavdevice.so libavutil.so)

catkin_simple()

# example boost usage
# find_package(Boost REQUIRED COMPONENTS system thread)

# C++0x support - not quite the same as final C++11!
# use carefully;  can interfere with point-cloud library
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# Libraries: uncomment the following and edit arguments to create a new library
# cs_add_library(my_lib src/my_lib.cpp)   

# Executables: uncomment the following and edit arguments to compile new nodes
# may add more of these lines for more nodes from the same package
# cs_add_executable(example src/example.cpp)
cs_add_executable(drag_test src/drag_test.cpp src/drag_rtmp.cpp)
cs_add_executable(drag_ros src/drag_ros.cpp src/drag_rtmp.cpp)

#the following is required, if desire to link a node in this package with a library created in this same package
# edit the arguments to reference the named node and named library within this package
# target_link_library(example my_lib)
target_link_libraries(drag_test ${OpenCV_LIBS} ${FFMPEG_LIBS})
target_link_libraries(drag_ros ${OpenCV_LIBS} ${FFMPEG_LIBS})

cs_install()
cs_export()
    