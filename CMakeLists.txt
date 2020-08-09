cmake_minimum_required(VERSION 2.8) 

project(SecondGear)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
    -D_PTHREADS
    -Wno-unused-parameter
)


set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")


string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

include_directories(${PROJECT_SOURCE_DIR}/utils)

set (SRCS 
    utils/CountDownLatch.cpp
    utils/Thread.cpp
    )

set (SRCS ${SRCS}
    EventLoop.cpp
    )

add_executable(main main.cpp ${SRCS})

target_link_libraries(main pthread)