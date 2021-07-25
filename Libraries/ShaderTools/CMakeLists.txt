cmake_minimum_required(VERSION 3.16)

project(ShaderTools)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_INCLUDES
    ${INCLUDE_DIR}/ShaderTools/ShaderLoader.h
    ${INCLUDE_DIR}/ShaderTools/ShaderReflection.h
)

set(SRCS
    ${SRC_DIR}/ShaderLoader.cpp
    ${SRC_DIR}/ShaderReflection.cpp
)

add_library(ShaderTools ${PUBLIC_INCLUDES} ${SRCS})

target_include_directories(ShaderTools PUBLIC ${INCLUDE_DIR} PRIVATE ${SRC_DIR})

target_link_libraries(ShaderTools PRIVATE Utils ShaderConductor)

target_include_directories(ShaderTools PRIVATE ${BMSAMP_EXTERNAL_DIR}/ShaderConductor/External/DirectXShaderCompiler/include)

add_custom_command(TARGET ShaderTools POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/$<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC_DIR}/hlsl.hlsl ${PACKAGE_DIR}/$<CONFIG>
)
