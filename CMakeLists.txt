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


if(DEFINED ENV{TARGET_XMX} OR TARGET_XMX)
    message("target : XMX, using compact ui, scale 1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTARGET_XMX=1 -DCOMPACT_UI_SCALE=1 -DFORCE_COMPACT_UI=1")
else()
    if (DEFINED ENV{ALWAYS_COMPACT_UI} OR ALWAYS_COMPACT_UI)
        set(ALWAYS_COMPACT_UI 1)
        message("target : SSP, forced compact ui, scale 1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTARGET_SSP=1 -DCOMPACT_UI_SCALE=1 -DFORCE_COMPACT_UI=1")
    else()
        message("target : SSP, ssp ui and scale")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTARGET_SSP=1 -DCOMPACT_UI_SCALE=2")
    endif() 
endif()

set(SOURCES
    src/PercCmd.cpp
    external/cJSON/cJSON.c
)

include_directories (
    external/cJSON
)


add_executable(PercCmd ${SOURCES})

target_link_libraries(PercCmd ${CMAKE_DL_LIBS} pthread)




