cmake_minimum_required(VERSION 3.16)

project(Buma3DHelpers)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_INCLUDES
    ${INCLUDE_DIR}/Buma3DHelpers/Buma3DHelpers.h
    ${INCLUDE_DIR}/Buma3DHelpers/B3DDescHash.h
    ${INCLUDE_DIR}/Buma3DHelpers/B3DDescHelpers.h
    ${INCLUDE_DIR}/Buma3DHelpers/B3DInit.h
    ${INCLUDE_DIR}/Buma3DHelpers/FormatUtils.h
)

set(SRCS
    ${SRC_DIR}/Buma3DHelpers.cpp
    ${SRC_DIR}/FormatUtils.cpp
)

add_library(Buma3DHelpers ${PUBLIC_INCLUDES} ${SRCS})

target_include_directories(Buma3DHelpers PUBLIC ${INCLUDE_DIR} PRIVATE ${SRC_DIR})

target_link_libraries(Buma3DHelpers PUBLIC Buma3D_Header Utils)
