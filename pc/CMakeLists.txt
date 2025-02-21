cmake_minimum_required(VERSION 3.1)

project(blodwen)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
set(SOURCES rover.cpp regsauto.cpp sim.cpp)


add_custom_target(blodwentar ALL
    DEPENDS blodwen
    COMMAND sh ${CMAKE_SOURCE_DIR}/buildLibrary ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
    
add_library(blodwen ${SOURCES})
