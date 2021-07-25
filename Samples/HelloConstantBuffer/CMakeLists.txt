cmake_minimum_required(VERSION 3.16)

set(TARGET HelloConstantBuffer)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(INC_DIRS )
set(SRCS )

set(SHADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/shader/ShaderHeader.hlsli
    ${CMAKE_CURRENT_SOURCE_DIR}/shader/VertexShader.hlsl
    ${CMAKE_CURRENT_SOURCE_DIR}/shader/PixelShader.hlsl
)
list(APPEND SRCS ${SHADERS})

make_sample_app(${TARGET} SRCS INC_DIRS)

if(MSVC)
    # 自動選択されたファイルプロパティをオーバーライドします
    set_source_files_properties(${SHADERS} PROPERTIES VS_TOOL_OVERRIDE "None")
endif(MSVC)

source_group(Shaders FILES ${SHADERS})

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shader ${PACKAGE_DIR}/$<CONFIG>/${TARGET}/shader
)
