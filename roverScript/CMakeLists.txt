cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(ROVER)

find_package(Readline REQUIRED)


set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
#variable_watch(ANGORTDIR)

# would be awesome if these could live in an angort cmake module

set(ANGORTDIR $ENV{HOME}/angort)
set(WORDFILELIST)

# this macro generates a custom build step which takes the files passed in,
# passes them through Angort's 'makeWords.pl' script, and compiles *those*.
# The files generated are added to WORDFILELIST, which should then be put
# into the SOURCES. So, Angort word files should be added with this.

macro(add_words_files)
    foreach(file ${ARGV})
        add_custom_command(
            OUTPUT ${file}.cpp
            DEPENDS ${file}
            COMMAND perl ${ANGORTDIR}/makeWords.pl ${CMAKE_SOURCE_DIR}/${file} >${file}.cpp
        )
        set(WORDFILELIST ${WORDFILELIST} ${file}.cpp)
    endforeach(file)
endmacro(add_words_files)

link_directories(${ANGORTDIR}/build/lib)
include_directories(${ANGORTDIR}/include ${READLINE_INCLUDE_DIR} ../pc)

include_directories(${CMAKE_SOURCE_DIR}) # so that words files can find them

    
add_words_files(wordsCalib.cpp wordsControl.cpp wordsUtil.cpp stdMath.cpp
    wordsUDP.cpp)

set(SOURCES main.cpp udpclient.cpp udpserver.cpp
    ../firmware/common/regsauto.cpp ../pc/rover.cpp ../pc/sim.cpp
    ${WORDFILELIST})

#add_executable(roverserver ${SOURCES})

add_executable(rover ${SOURCES})
set_target_properties(rover PROPERTIES COMPILE_FLAGS "-DREADLINE")

#target_link_libraries(roverserver angort rt m pthread ${READLINE_LIBRARY})
target_link_libraries(rover angort rt m pthread dl ${READLINE_LIBRARY})


