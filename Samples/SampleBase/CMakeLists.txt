cmake_minimum_required(VERSION 3.16)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_INCLUDES
    ${INCLUDE_DIR}/SampleBase/SampleBase.h
)

set(SRCS
    ${SRC_DIR}/SampleBase.cpp
)

add_library(SampleBase ${PUBLIC_INCLUDES} ${SRCS})

target_include_directories(SampleBase PUBLIC ${INCLUDE_DIR} PRIVATE ${SRC_DIR})

target_link_libraries(SampleBase PUBLIC glm::glm AppFramework DeviceResources TextureLoads ShaderTools)
