cmake_minimum_required(VERSION 3.16)

project(TextureLoads)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_INCLUDES
    ${INCLUDE_DIR}/TextureLoads/TextureLoads.h
)

set(SRCS
    ${SRC_DIR}/TextureLoads.cpp
)

add_library(TextureLoads ${PUBLIC_INCLUDES} ${SRCS})

target_include_directories(TextureLoads PUBLIC ${INCLUDE_DIR} PRIVATE ${SRC_DIR})

target_link_libraries(TextureLoads PRIVATE stb)
