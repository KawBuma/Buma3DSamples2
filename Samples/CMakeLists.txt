cmake_minimum_required(VERSION 3.16)

# ビルドされたサンプルを統合するディレクトリ
set(PACKAGE_DIR ${CMAKE_BINARY_DIR}/built)

# サンプルアプリケーションの実行可能ファイル
add_subdirectory(${BMSAMP_SAMPLES_DIR}/Buma3DSamples)

# サンプルアプリケーション共通のBuma3Dオブジェクトを管理します
add_subdirectory(${BMSAMP_SAMPLES_DIR}/SampleBase)

# サンプルアプリケーションのプロジェクトを作成します
function(make_sample_app _TGT_NAME _ADDITIONAL_FILES _ADDITIONAL_INC_DIRS)
    # NOTE: CMAKE_CURRENT_SOURCE_DIR には関数の呼び出し元を基準にした値が設定されます
    set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(FILES ${INCLUDE_DIR}/${_TGT_NAME}/${_TGT_NAME}.h ${SRC_DIR}/${_TGT_NAME}.cpp)

    add_library(${_TGT_NAME} MODULE ${FILES} ${${_ADDITIONAL_FILES}})
    target_include_directories(${_TGT_NAME} PRIVATE ${INCLUDE_DIR} ${SRC_DIR} ${${_ADDITIONAL_INC_DIRS}})
    target_link_libraries(${_TGT_NAME} PRIVATE SampleBase)
    target_compile_definitions(${_TGT_NAME} PRIVATE BUMA_APP_EXPORT)

    add_dependencies(Buma3DSamples ${_TGT_NAME})

    # ビルドしたサンプルをPACKAGE_DIRへ複製
    add_custom_command(TARGET ${_TGT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/$<CONFIG>/${_TGT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${_TGT_NAME}> ${PACKAGE_DIR}/$<CONFIG>/${_TGT_NAME}
    )
endfunction()

# サンプルアプリケーションを追加
add_subdirectory(${BMSAMP_SAMPLES_DIR}/HelloTriangle)
add_subdirectory(${BMSAMP_SAMPLES_DIR}/HelloConstantBuffer)
# add_subdirectory(${BMSAMP_SAMPLES_DIR}/HelloTexture)
# add_subdirectory(${BMSAMP_SAMPLES_DIR}/HelloImGui)
# add_subdirectory(${BMSAMP_SAMPLES_DIR}/SceneRendering)
