cmake_minimum_required(VERSION 3.16)

# 以下のライブラリはCMakeListsが既に用意されています
add_subdirectory(${BMSAMP_EXTERNAL_DIR}/glm) # uninstall
add_subdirectory(${BMSAMP_EXTERNAL_DIR}/spdlog)
add_subdirectory(${BMSAMP_EXTERNAL_DIR}/SDL)
add_subdirectory(${BMSAMP_EXTERNAL_DIR}/json)
add_subdirectory(${BMSAMP_EXTERNAL_DIR}/ShaderConductor)

set_target_properties(glm           PROPERTIES FOLDER External)
set_target_properties(uninstall     PROPERTIES FOLDER External)
set_target_properties(spdlog        PROPERTIES FOLDER External)
set_target_properties(nlohmann_json PROPERTIES FOLDER External)

set_nest_folder_property_in(${BMSAMP_EXTERNAL_DIR}/SDL             External/SDL)
set_nest_folder_property_in(${BMSAMP_EXTERNAL_DIR}/ShaderConductor External/ShaderConductor)

# ===============================================================================================
# imgui
# ===============================================================================================
add_library(imgui
    ${BMSAMP_EXTERNAL_DIR}/imgui/imconfig.h
    ${BMSAMP_EXTERNAL_DIR}/imgui/imgui_demo.cpp
    ${BMSAMP_EXTERNAL_DIR}/imgui/imgui_draw.cpp
    ${BMSAMP_EXTERNAL_DIR}/imgui/imgui_internal.h
    ${BMSAMP_EXTERNAL_DIR}/imgui/imgui_tables.cpp
    ${BMSAMP_EXTERNAL_DIR}/imgui/imgui_widgets.cpp
    ${BMSAMP_EXTERNAL_DIR}/imgui/imgui.cpp
    ${BMSAMP_EXTERNAL_DIR}/imgui/imgui.h
    ${BMSAMP_EXTERNAL_DIR}/imgui/imstb_rectpack.h
    ${BMSAMP_EXTERNAL_DIR}/imgui/imstb_textedit.h
    ${BMSAMP_EXTERNAL_DIR}/imgui/imstb_truetype.h
    ${BMSAMP_EXTERNAL_DIR}/imgui/misc/cpp/imgui_stdlib.h
    ${BMSAMP_EXTERNAL_DIR}/imgui/misc/cpp/imgui_stdlib.cpp

    ${BMSAMP_EXTERNAL_DIR}/imgui/backends/imgui_impl_sdl.h
    ${BMSAMP_EXTERNAL_DIR}/imgui/backends/imgui_impl_sdl.cpp
)
if(MSVC)
    target_sources(imgui PRIVATE $<${MSVC}:${BMSAMP_EXTERNAL_DIR}/imgui/misc/debuggers/imgui.natvis>)
endif(MSVC)
target_include_directories(imgui PUBLIC ${BMSAMP_EXTERNAL_DIR}/imgui)
target_link_libraries(imgui PRIVATE SDL2)
set_target_properties(imgui PROPERTIES FOLDER External)


# ===============================================================================================
# stb
# ===============================================================================================
add_library(stb INTERFACE
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_c_lexer.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_connected_components.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_divide.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_ds.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_dxt.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_easy_font.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_herringbone_wang_tile.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_hexwave.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_image_resize.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_image_write.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_image.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_include.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_leakcheck.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_perlin.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_rect_pack.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_sprintf.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_textedit.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_tilemap_editor.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_truetype.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_vorbis.c
    ${BMSAMP_EXTERNAL_DIR}/stb/stb_voxel_render.h
    ${BMSAMP_EXTERNAL_DIR}/stb/stb.h
)
target_include_directories(stb INTERFACE ${BMSAMP_EXTERNAL_DIR}/stb)
set_target_properties(stb PROPERTIES FOLDER External)
