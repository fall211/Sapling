cmake_minimum_required(VERSION 3.10)

if(NOT PROJECT_NAME)
    project(Sapling)
endif()

set(CMAKE_CXX_STANDARD 20)

# paths
set(SAPLING_ROOT    ${CMAKE_CURRENT_SOURCE_DIR})
set(SAPLING_INCLUDE ${SAPLING_ROOT}/include)
set(SAPLING_SRC     ${SAPLING_ROOT}/src)
set(THIRD_PARTY     ${SAPLING_ROOT}/third_party)

set(FMOD_ROOT       ${THIRD_PARTY}/fmod)
set(FMOD_CORE_INC   ${FMOD_ROOT}/core/inc)
set(FMOD_STUDIO_INC ${FMOD_ROOT}/studio/inc)
set(FMOD_CORE_LIB   ${FMOD_ROOT}/core/lib)
set(FMOD_STUDIO_LIB ${FMOD_ROOT}/studio/lib)

# engine src files
set(SAPLING_SOURCES
    ${SAPLING_SRC}/Core/Engine.cpp
    ${SAPLING_SRC}/Core/Scene.cpp
    ${SAPLING_SRC}/Core/AssetManager.cpp
    ${SAPLING_SRC}/Core/AudioEngine.cpp
    ${SAPLING_SRC}/Core/Input.cpp
    ${SAPLING_SRC}/Core/Logger.cpp
    ${SAPLING_SRC}/Core/PrefabLoader.cpp
    ${SAPLING_SRC}/ECS/Entity.cpp
    ${SAPLING_SRC}/ECS/EntityManager.cpp
    ${SAPLING_SRC}/ECS/Component.cpp
    ${SAPLING_SRC}/Renderer/Sprout.cpp
    ${SAPLING_SRC}/Renderer/Window.cpp
    ${SAPLING_SRC}/Renderer/Texture.cpp
    ${SAPLING_SRC}/Renderer/Font.cpp

    ${SAPLING_SRC}/Renderer/Mesh.cpp
    ${SAPLING_SRC}/Renderer/Material.cpp
    ${SAPLING_SRC}/Renderer/Forward3DPass.cpp
    ${SAPLING_SRC}/Renderer/Quad2DPass.cpp
    ${SAPLING_SRC}/Renderer/Skeleton.cpp
    ${SAPLING_SRC}/Renderer/Animation.cpp
    ${SAPLING_SRC}/Utility/Physics.cpp

    ${THIRD_PARTY}/ufbx/ufbx.c
)

# macOS only objective c metal backend
if(APPLE)
    list(APPEND SAPLING_SOURCES ${SAPLING_SRC}/Renderer/SproutMetal.mm)
endif()

# editor (optional)
option(SAPLING_EDITOR "Build with prefab editor (requires Dear ImGui)" OFF)

if(SAPLING_EDITOR)
    list(APPEND SAPLING_SOURCES

        ${THIRD_PARTY}/imgui/imgui.cpp
        ${THIRD_PARTY}/imgui/imgui_draw.cpp
        ${THIRD_PARTY}/imgui/imgui_widgets.cpp
        ${THIRD_PARTY}/imgui/imgui_tables.cpp

        ${SAPLING_SRC}/Editor/ImGuiContext.cpp
        ${SAPLING_SRC}/Editor/PrefabEditor.cpp
        ${SAPLING_SRC}/Editor/ComponentInspector.cpp
    )
endif()

# engine inc files
set(SAPLING_INCLUDE_DIRS
    ${SAPLING_INCLUDE}

    ${THIRD_PARTY}
    ${THIRD_PARTY}/stb
    ${THIRD_PARTY}/ufbx

    ${FMOD_CORE_INC}
    ${FMOD_STUDIO_INC}
)

if(SAPLING_EDITOR)
    list(APPEND SAPLING_INCLUDE_DIRS ${THIRD_PARTY}/imgui)
endif()

# compile defs
set(SAPLING_COMPILE_DEFS)
if(APPLE OR WIN32)
    list(APPEND SAPLING_COMPILE_DEFS GLM_FORCE_DEPTH_ZERO_TO_ONE)
endif()

if(SAPLING_EDITOR)
    list(APPEND SAPLING_COMPILE_DEFS SAPLING_HAS_EDITOR)
endif()

# link external libs
set(SAPLING_LINK_LIBS)
if(APPLE)
    list(APPEND SAPLING_LINK_LIBS
        "-framework Cocoa"
        "-framework QuartzCore"
        "-framework Metal"
        "-framework MetalKit"
        "${FMOD_CORE_LIB}/libfmod.dylib"
        "${FMOD_STUDIO_LIB}/libfmodstudio.dylib"
    )
    set(SAPLING_FMOD_DYLIBS
        "${FMOD_CORE_LIB}/libfmod.dylib"
        "${FMOD_STUDIO_LIB}/libfmodstudio.dylib"
    )
elseif(WIN32)
    set(FMOD_CORE_LIB_WIN  ${FMOD_CORE_LIB}/x86)
    set(FMOD_STUDIO_LIB_WIN ${FMOD_STUDIO_LIB}/x86)
    list(APPEND SAPLING_LINK_LIBS
        "${FMOD_CORE_LIB_WIN}/fmod_vc.lib"
        "${FMOD_STUDIO_LIB_WIN}/fmodstudio_vc.lib"
    )
    set(SAPLING_FMOD_DLLS
        "${FMOD_CORE_LIB_WIN}/fmod.dll"
        "${FMOD_STUDIO_LIB_WIN}/fmodstudio.dll"
    )
endif()
