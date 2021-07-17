cmake_minimum_required(VERSION 3.16)

project(Buma3DSamples)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(Buma3DSamples ${SRC_DIR}/Buma3DSamples.cpp)

target_link_libraries(Buma3DSamples PUBLIC AppFramework)

if(MSVC)
    set_property(TARGET Buma3DSamples PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PACKAGE_DIR}/$<CONFIG>)
endif(MSVC)

add_custom_command(TARGET Buma3DSamples POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:dxcompiler> $<TARGET_FILE_DIR:Buma3DSamples>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ShaderConductor> $<TARGET_FILE_DIR:Buma3DSamples>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL2> $<TARGET_FILE_DIR:Buma3DSamples>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:spdlog> $<TARGET_FILE_DIR:Buma3DSamples>
)

if(BMSAMP_USE_D3D12)
    # NOTE: DXILへのコンパイルには、ワーキングディレクトリに dxil.dll が必要です。
    # https://github.com/microsoft/DirectXShaderCompiler/issues/2550
    # https://asawicki.info/news_1719_two_shader_compilers_of_direct3d_12#:~:text=3.%20New%20Compiler%20-%20Offline
    set(DXIL_PATH "C:/Program Files (x86)/Windows Kits/10/Redist/D3D/x64/dxil.dll")
    add_custom_command(TARGET Buma3DSamples POST_BUILD
        DEPENDS Buma3D_D3D12
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DXIL_PATH} $<TARGET_FILE_DIR:Buma3DSamples>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WinPixEventRuntime_DLL_PATH} $<TARGET_FILE_DIR:Buma3DSamples>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Buma3D_D3D12> $<TARGET_FILE_DIR:Buma3DSamples>
    )
endif(BMSAMP_USE_D3D12)

if(BMSAMP_USE_VULKAN)
    add_custom_command(TARGET Buma3DSamples POST_BUILD
        DEPENDS Buma3D_Vulkan
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Buma3D_Vulkan> $<TARGET_FILE_DIR:Buma3DSamples>
    )
endif(BMSAMP_USE_VULKAN)

add_custom_command(TARGET Buma3DSamples POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:Buma3DSamples> ${PACKAGE_DIR}/$<CONFIG>
)
