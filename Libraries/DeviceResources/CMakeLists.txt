cmake_minimum_required(VERSION 3.16)

project(DeviceResources)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_INCLUDES
    ${INCLUDE_DIR}/DeviceResources/CommandListChain.h
    ${INCLUDE_DIR}/DeviceResources/CommandQueue.h
    ${INCLUDE_DIR}/DeviceResources/CopyContext.h
    ${INCLUDE_DIR}/DeviceResources/DeviceResources.h
    ${INCLUDE_DIR}/DeviceResources/Resource.h
    ${INCLUDE_DIR}/DeviceResources/ResourceBuffer.h
    ${INCLUDE_DIR}/DeviceResources/ResourceTexture.h
    ${INCLUDE_DIR}/DeviceResources/ShaderToolsConv.h
    ${INCLUDE_DIR}/DeviceResources/SwapChain.h
)

set(PRIVATE_INCLUDES
    ${INCLUDE_DIR}/DeviceResources/private/ResourceViewCache.h
)

set(SRCS
    ${SRC_DIR}/CommandQueue.cpp
    ${SRC_DIR}/CopyContext.cpp
    ${SRC_DIR}/DeviceResources.cpp
    ${SRC_DIR}/Resource.cpp
    ${SRC_DIR}/ResourceBuffer.cpp
    ${SRC_DIR}/ResourceHeapAllocator.cpp
    ${SRC_DIR}/ResourceHeapAllocator.h
    ${SRC_DIR}/ResourceHeapProperties.h
    ${SRC_DIR}/ResourceTexture.cpp
    ${SRC_DIR}/StagingBufferPool.cpp
    ${SRC_DIR}/StagingBufferPool.h
    ${SRC_DIR}/SwapChain.cpp
    ${SRC_DIR}/VariableSizeAllocationsManager.cpp
    ${SRC_DIR}/VariableSizeAllocationsManager.h
)

add_library(DeviceResources ${PUBLIC_INCLUDES} ${PRIVATE_INCLUDES} ${SRCS})

source_group(Include FILES ${PUBLIC_INCLUDES})

target_include_directories(DeviceResources PUBLIC ${INCLUDE_DIR} PRIVATE ${SRC_DIR})

target_link_libraries(DeviceResources PUBLIC Buma3D_Header Buma3DHelpers PRIVATE Utils)
