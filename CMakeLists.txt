cmake_minimum_required(VERSION 3.16)

# ===============================================================================================
# プロジェクトの定義
# ===============================================================================================
project(Buma3DSamples2)

# ===============================================================================================
# オプションの定義
# ===============================================================================================
option(BMSAMP_USE_D3D12 "Whether to use d3d12 in rendering backend" ON)
option(BMSAMP_USE_VULKAN "Whether to use vulkan in rendering backend" ON)

# ===============================================================================================
# 共通パスの定義
# ===============================================================================================
set(BMSAMP_EXTERNAL_DIR  ${CMAKE_SOURCE_DIR}/External)
set(BMSAMP_LIBRARY_DIR   ${CMAKE_SOURCE_DIR}/Libraries)
set(BMSAMP_SAMPLES_DIR   ${CMAKE_SOURCE_DIR}/Samples)

# ビルドされたサンプルを統合するディレクトリ
set(PACKAGE_DIR ${CMAKE_BINARY_DIR}/built)

# ===============================================================================================
# コンパイルオプションの伝播ライブラリ
# ===============================================================================================
add_library(bmsamp_compiler_flags INTERFACE)
target_compile_features(bmsamp_compiler_flags INTERFACE cxx_std_17)

if(MSVC)
    target_compile_options(bmsamp_compiler_flags         INTERFACE "/W3")                    # 警告レベル
    target_compile_options(bmsamp_compiler_flags         INTERFACE "/MP")                    # 複数プロセッサによるコンパイル
    target_compile_options(bmsamp_compiler_flags         INTERFACE "/source-charset:utf-8")  # ソースファイルの文字セットはutf-8です
    target_compile_definitions(bmsamp_compiler_flags     INTERFACE _UNICODE UNICODE)         # これにより、 文字セット: "Unicode 文字セットを使用する" が自動的に選択されます
endif(MSVC)

# ===============================================================================================
# フォルダの設定関数
# ===============================================================================================
function(get_all_targets_in DIR RESULT)
    get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
    foreach(SD IN LISTS SUBDIRS)
        get_all_targets_in("${SD}" ${RESULT})
    endforeach()
    get_property(SUBTGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    set(${RESULT} ${${RESULT}} ${SUBTGTS} PARENT_SCOPE)
endfunction()

function(set_nest_folder_property_in DIR NEST_FOLDERS)
    get_all_targets_in(${DIR} TARGETS)
    foreach(T IN LISTS TARGETS)
        get_target_property(F ${T} FOLDER)
        if(F)
            set_target_properties(${T} PROPERTIES FOLDER "${NEST_FOLDERS}/${F}")
        else()
            set_target_properties(${T} PROPERTIES FOLDER "${NEST_FOLDERS}")
        endif()
    endforeach()
endfunction()

# ===============================================================================================
# サブディレクトリの追加
# ===============================================================================================
# サードパーティー製ライブラリ

if(NOT EXISTS SPDLOG_BUILD_SHARED OR NOT SPDLOG_BUILD_SHARED)
    # message(FATAL_ERROR "Buma3DSamples2 shares the spdlog::default_logger instance, so please set the SPDLOG_BUILD_SHARED option to ON")
    message(WARNING "Buma3DSamples2 forced SPDLOG_BUILD_SHARED to ON")
    set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Build shared library (*FORCE ON from Buma3DSamples2*)" FORCE)
endif(NOT SPDLOG_BUILD_SHARED)

add_subdirectory(${BMSAMP_EXTERNAL_DIR})

# Buma3D
add_subdirectory(${BMSAMP_EXTERNAL_DIR}/Buma3D)
set_nest_folder_property_in(${BMSAMP_EXTERNAL_DIR}/Buma3D Buma3D)

# これ以降のすべてのライブラリにリンクします
link_libraries(bmsamp_compiler_flags)

# Buma3DSamples2 のサブプロジェクト
add_subdirectory(${BMSAMP_LIBRARY_DIR})
set_nest_folder_property_in(${BMSAMP_LIBRARY_DIR} Libraries)

# メインのサンプルプログラム
add_subdirectory(${BMSAMP_SAMPLES_DIR})
set_nest_folder_property_in(${BMSAMP_SAMPLES_DIR} Samples)

if(MSVC)
    # F5を押してすぐに開始できるようにします
    set_directory_properties(PROPERTIES VS_STARTUP_PROJECT Buma3DSamples)
endif(MSVC)


# ===============================================================================================
# TODO: インストール
# ===============================================================================================
