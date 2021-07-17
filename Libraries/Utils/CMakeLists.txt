cmake_minimum_required(VERSION 3.16)

project(Utils)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_INCLUDES
    ${INCLUDE_DIR}/Utils/Compiler.h
    ${INCLUDE_DIR}/Utils/Definitions.h
    ${INCLUDE_DIR}/Utils/LazyDelegate.h
    ${INCLUDE_DIR}/Utils/Logger.h
    ${INCLUDE_DIR}/Utils/NonCopyable.h
    ${INCLUDE_DIR}/Utils/StepTimer.h
    ${INCLUDE_DIR}/Utils/Utils.h
)

set(PRIVATE_INCLUDES
    ${INCLUDE_DIR}/Utils/private/LoggerDetails.h
)

set(SRCS
    ${SRC_DIR}/LazyDelegate.cpp
    ${SRC_DIR}/Logger.cpp
    ${SRC_DIR}/StepTimer.cpp
    ${SRC_DIR}/Utils.cpp
)

add_library(Utils ${PUBLIC_INCLUDES} ${PRIVATE_INCLUDES} ${SRCS})

target_include_directories(Utils PUBLIC ${INCLUDE_DIR} PRIVATE ${SRC_DIR})

target_link_libraries(Utils PUBLIC spdlog)
