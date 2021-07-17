cmake_minimum_required(VERSION 3.16)

project(AppFramework)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_INCLUDES
    ${INCLUDE_DIR}/AppFramework/Application.h
    ${INCLUDE_DIR}/AppFramework/EventInterfaces.h
    ${INCLUDE_DIR}/AppFramework/Framework.h
    ${INCLUDE_DIR}/AppFramework/Platform.h
    ${INCLUDE_DIR}/AppFramework/Window.h
)

set(SRCS
    ${SRC_DIR}/Application.cpp
    ${SRC_DIR}/Framework.cpp
    ${SRC_DIR}/Platform.cpp
    ${SRC_DIR}/Window.cpp
    )

set(PLATFORM_SRCS
    ${SRC_DIR}/Platform/PlatformSDL.cpp
    ${SRC_DIR}/Platform/WindowSDL.cpp
    ${SRC_DIR}/Platform/PlatformSDL.h
    ${SRC_DIR}/Platform/WindowSDL.h
)
# if(WIN32)
#     list(APPEND PLATFORM_SRCS
#         ${SRC_DIR}/Platform/PlatformWindows.cpp
#         ${SRC_DIR}/Platform/WindowWindows.cpp
#         ${SRC_DIR}/Platform/PlatformWindows.h
#         ${SRC_DIR}/Platform/WindowWindows.h
#     )
# endif()

list(APPEND SRCS ${PLATFORM_SRCS})

add_library(AppFramework ${PUBLIC_INCLUDES} ${SRCS})

source_group("Include" FILES ${PUBLIC_INCLUDES})
source_group("Source Files/Platform" FILES ${PLATFORM_SRCS})

target_include_directories(AppFramework PUBLIC ${INCLUDE_DIR} PRIVATE ${SRC_DIR})

target_link_libraries(AppFramework
    PUBLIC
        Utils
        SDL2

    PRIVATE
        glm::glm
        nlohmann_json::nlohmann_json
)
