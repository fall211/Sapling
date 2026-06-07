cmake_minimum_required(VERSION 3.10)
project(TechDemo)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# ────────────────────────────────────────────────────────
# ENGINE PATHS
# ────────────────────────────────────────────────────────
# The engine lives two directories up from Examples/TechDemo
set(SAPLING_ROOT    "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(SAPLING_INCLUDE "${SAPLING_ROOT}/include")
set(SAPLING_SRC     "${SAPLING_ROOT}/src")
set(THIRD_PARTY     "${SAPLING_ROOT}/third_party")

set(FMOD_ROOT       "${THIRD_PARTY}/fmod")
set(FMOD_CORE_INC   "${FMOD_ROOT}/core/inc")
set(FMOD_STUDIO_INC "${FMOD_ROOT}/studio/inc")
set(FMOD_CORE_LIB   "${FMOD_ROOT}/core/lib")
set(FMOD_STUDIO_LIB "${FMOD_ROOT}/studio/lib")

# ────────────────────────────────────────────────────────
# ENGINE SOURCE FILES
# ────────────────────────────────────────────────────────
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

if(APPLE)
    list(APPEND SAPLING_SOURCES ${SAPLING_SRC}/Renderer/SproutMetal.mm)
endif()

# ────────────────────────────────────────────────────────
# ENGINE INCLUDE DIRECTORIES
# ────────────────────────────────────────────────────────
set(SAPLING_INCLUDE_DIRS
    ${SAPLING_INCLUDE}
    ${THIRD_PARTY}
    ${THIRD_PARTY}/stb
    ${FMOD_CORE_INC}
    ${FMOD_STUDIO_INC}
)

# ────────────────────────────────────────────────────────
# ENGINE LINK LIBRARIES
# ────────────────────────────────────────────────────────
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

# ────────────────────────────────────────────────────────
# BUILD ENGINE AS STATIC LIBRARY
# ────────────────────────────────────────────────────────
add_library(SaplingEngine STATIC ${SAPLING_SOURCES})
target_include_directories(SaplingEngine PUBLIC ${SAPLING_INCLUDE_DIRS})
target_link_libraries(SaplingEngine PUBLIC ${SAPLING_LINK_LIBS})

# Metal and D3D11 use [0,1] depth range; tell GLM to match
if(APPLE OR WIN32)
    target_compile_definitions(SaplingEngine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
endif()

if(APPLE)
    set_source_files_properties(
        ${SAPLING_SRC}/Renderer/SproutMetal.mm
        PROPERTIES COMPILE_FLAGS "-x objective-c++"
    )
endif()

# ────────────────────────────────────────────────────────
# GAME SOURCE FILES
# ────────────────────────────────────────────────────────
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Scenes/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Scenes/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Systems/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/*.hpp"
)

# ────────────────────────────────────────────────────────
# CREATE GAME EXECUTABLE
# ────────────────────────────────────────────────────────
add_executable(TechDemo ${GAME_SOURCES})

target_link_libraries(TechDemo PRIVATE SaplingEngine)

target_include_directories(TechDemo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Scenes
    ${CMAKE_CURRENT_SOURCE_DIR}/Systems
    ${CMAKE_CURRENT_SOURCE_DIR}/Components
)

# ────────────────────────────────────────────────────────
# DEBUG BUILD: Define ASSETS_PATH and copy assets
# ────────────────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assets")
    target_compile_definitions(TechDemo PRIVATE
        DEBUG
        ASSETS_PATH="${ASSETS_DIR}/"
    )
    message(STATUS "Debug build - ASSETS_PATH set to: ${ASSETS_DIR}/")

    # Copy assets to debug output directory
    add_custom_command(TARGET TechDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:TechDemo>/Assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}" "$<TARGET_FILE_DIR:TechDemo>/Assets"
        COMMENT "Copying assets to debug output directory"
    )

    # Copy FMOD dylibs next to executable (macOS)
    if(APPLE)
        foreach(dylib ${SAPLING_FMOD_DYLIBS})
            add_custom_command(TARGET TechDemo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dylib}" $<TARGET_FILE_DIR:TechDemo>
                COMMENT "Copying ${dylib} to debug output"
            )
        endforeach()
        set_target_properties(TechDemo PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "@executable_path"
        )
    elseif(WIN32)
        foreach(dll ${SAPLING_FMOD_DLLS})
            add_custom_command(TARGET TechDemo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll}" $<TARGET_FILE_DIR:TechDemo>
                COMMENT "Copying ${dll} to debug output"
            )
        endforeach()
    endif()
endif()

# ────────────────────────────────────────────────────────
# RELEASE BUILD
# ────────────────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assets")

    if(APPLE)
        set(APP_DIR "${CMAKE_BINARY_DIR}/TechDemo.app")
        set(MACOS_DIR "${APP_DIR}/Contents/MacOS")
        set(RESOURCES_DIR "${APP_DIR}/Contents/Resources")

        set_target_properties(TechDemo PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${MACOS_DIR}"
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "@executable_path"
        )

        foreach(dylib ${SAPLING_FMOD_DYLIBS})
            add_custom_command(TARGET TechDemo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dylib}" "${MACOS_DIR}"
                COMMENT "Copying ${dylib} to bundle"
            )
        endforeach()

        add_custom_command(TARGET TechDemo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCES_DIR}/Assets"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}" "${RESOURCES_DIR}/Assets"
            COMMENT "Copying assets to bundle Resources/Assets"
        )

        file(WRITE "${APP_DIR}/Contents/Info.plist"
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>CFBundleExecutable</key>
    <string>TechDemo</string>
    <key>CFBundleName</key>
    <string>Sapling TechDemo</string>
    <key>CFBundleIdentifier</key>
    <string>com.sapling.TechDemo</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
</dict>
</plist>
")
        message(STATUS "Release build - creating .app bundle at: ${APP_DIR}")

    elseif(WIN32)
        foreach(dll ${SAPLING_FMOD_DLLS})
            add_custom_command(TARGET TechDemo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll}" $<TARGET_FILE_DIR:TechDemo>
                COMMENT "Copying ${dll} to output directory"
            )
        endforeach()
    endif()
endif()
