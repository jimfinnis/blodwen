cmake_minimum_required(VERSION 2.8.12)
project(blodwen)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
find_package(pybind11 REQUIRED)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../regsauto.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/../rover.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/../sim.cpp)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../comms.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../drive.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../hwconfig.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../lift.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../master.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../motor.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../motordata.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../motorsim.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../regconfig.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../regs.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../regsauto.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../rover.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../roverexcept.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../sim.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../slave.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../status.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../steer.h)

pybind11_add_module(blodwen blodwen.cpp ${SOURCES} ${HEADERS})
