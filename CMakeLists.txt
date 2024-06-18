cmake_minimum_required(VERSION 3.15)
project(PercCmd)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message("apple config")
    find_package(SDL2 REQUIRED)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (CMAKE_CROSSCOMPILING)
        message("cross compile config")
    else ()
            message("no cross compile config")
    endif ()
endif ()

set(SOURCES
    src/PercCmd.cpp
    external/cJSON/cJSON.c
)

include_directories (
    external/cJSON
)


add_executable(PercCmd ${SOURCES})

target_link_libraries(PercCmd ${CMAKE_DL_LIBS} pthread)




