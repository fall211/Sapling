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

if(UNIX AND NOT APPLE)
    set(FMOD_CORE_INC   ${THIRD_PARTY}/fmod_stub)
    set(FMOD_STUDIO_INC ${THIRD_PARTY}/fmod_stub)
endif()

# engine src files
set(SAPLING_SOURCES
    ${SAPLING_SRC}/Core/Engine.cpp
    ${SAPLING_SRC}/Core/Scene.cpp
    ${SAPLING_SRC}/Core/AssetManager.cpp
    ${SAPLING_SRC}/Core/AudioEngine.cpp
    ${SAPLING_SRC}/Core/Input.cpp
    ${SAPLING_SRC}/Core/Logger.cpp
    ${SAPLING_SRC}/Core/ManifestLoader.cpp
    ${SAPLING_SRC}/ECS/Entity.cpp
    ${SAPLING_SRC}/ECS/EntityManager.cpp
    ${SAPLING_SRC}/ECS/Component.cpp
    ${SAPLING_SRC}/Renderer/Sprout.cpp
    ${SAPLING_SRC}/Renderer/Window.cpp
    ${SAPLING_SRC}/Renderer/Texture.cpp
    ${SAPLING_SRC}/Renderer/Font.cpp
    ${SAPLING_SRC}/Renderer/Quad2DPass.cpp
    ${SAPLING_SRC}/Utility/Physics.cpp
)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND SAPLING_SOURCES ${SAPLING_SRC}/Core/AgentCli.cpp)
    list(APPEND SAPLING_COMPILE_DEFS SAPLING_AGENT_CLI)
endif()

# macOS only objective c metal backend
if(APPLE)
    list(APPEND SAPLING_SOURCES ${SAPLING_SRC}/Renderer/SproutMetal.mm)
endif()

# engine inc files
set(SAPLING_INCLUDE_DIRS
    ${SAPLING_INCLUDE}

    ${THIRD_PARTY}
    ${THIRD_PARTY}/stb

    ${FMOD_CORE_INC}
    ${FMOD_STUDIO_INC}
)

# compile defs
set(SAPLING_COMPILE_DEFS)
if(APPLE OR WIN32)
    list(APPEND SAPLING_COMPILE_DEFS GLM_FORCE_DEPTH_ZERO_TO_ONE)
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
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    list(APPEND SAPLING_LINK_LIBS
        OpenGL::GL
        ${X11_LIBRARIES}
        Threads::Threads
        dl
        m
    )
    if(X11_Xcursor_LIB)
        list(APPEND SAPLING_LINK_LIBS ${X11_Xcursor_LIB})
    else()
        list(APPEND SAPLING_LINK_LIBS Xcursor)
    endif()
    if(X11_Xi_LIB)
        list(APPEND SAPLING_LINK_LIBS ${X11_Xi_LIB})
    else()
        list(APPEND SAPLING_LINK_LIBS Xi)
    endif()
endif()
