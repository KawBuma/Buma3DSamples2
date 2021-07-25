cmake_minimum_required(VERSION 3.16)

project(MyImgui)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_INCLUDES
    ${INCLUDE_DIR}/MyImgui/MyImgui.h
)

set(SRCS
    ${SRC_DIR}/MyImgui.cpp
    ${SRC_DIR}/MyImguiRenderer.cpp
    ${SRC_DIR}/MyImguiRenderer.h
    ${SRC_DIR}/Utils.h

    ${SRC_DIR}/Imgui_PS.hlsl
    ${SRC_DIR}/Imgui_VS.hlsl
)

if(MSVC)
    # 自動選択されたファイルプロパティをオーバーライドします
    set_source_files_properties(
        ${SRC_DIR}/Imgui_PS.hlsl
        ${SRC_DIR}/Imgui_VS.hlsl
        PROPERTIES VS_TOOL_OVERRIDE "None"
    )
endif(MSVC)

add_library(MyImgui ${PUBLIC_INCLUDES} ${SRCS})

source_group(Shaders
    ${SRC_DIR}/Imgui_PS.hlsl
    ${SRC_DIR}/Imgui_VS.hlsl
)

target_include_directories(MyImgui PUBLIC ${INCLUDE_DIR} PRIVATE ${SRC_DIR})

target_link_libraries(MyImgui
    PUBLIC
        imgui
        Buma3D_Header
        DeviceResources
    PRIVATE
        SDL2
        Utils
        Buma3DHelpers
        ShaderTools
        AppFramework
)
