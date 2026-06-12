cmake_minimum_required(VERSION 3.26)
project(Minigin)

# --------------------- Source files ---------------------
add_library(Minigin STATIC
    
    Font.cpp
    
    GameObject.cpp
    InputManager.cpp
    Minigin.cpp
    RenderComponent.cpp
    Renderer.cpp
    ResourceManager.cpp
    Scene.cpp
    SceneManager.cpp
    ServiceLocator.cpp
    SoundSystemImpl.cpp
    TextComponent.cpp
    Texture2D.cpp

    # Platform-specific Gamepad
    $<$<NOT:$<BOOL:${EMSCRIPTEN}>>:Gamepad.cpp>
    $<$<BOOL:${EMSCRIPTEN}>:Gamepad_SDL.cpp>
)

target_compile_features(Minigin PUBLIC cxx_std_20)

# --------------------- Dependencies ---------------------
include(FetchContent)

# glm
find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
    FetchContent_Declare(
        glm
        URL https://github.com/g-truc/glm/releases/download/1.0.3/glm-1.0.3.zip
        DOWNLOAD_NO_PROGRESS ON
    )
    FetchContent_MakeAvailable(glm)
endif()

# SDL3
find_package(SDL3 3.4 CONFIG QUIET)
if(NOT SDL3_FOUND)
    if (WIN32)
        FetchContent_Declare(
            SDL3
            URL https://www.libsdl.org/release/SDL3-devel-3.4.0-VC.zip
            DOWNLOAD_NO_PROGRESS ON
        )
        FetchContent_MakeAvailable(SDL3)
        list(PREPEND CMAKE_PREFIX_PATH "${sdl3_SOURCE_DIR}")
        find_package(SDL3 CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.4.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(SDL3)
    endif()
endif()

# SDL3_ttf
find_package(SDL3_ttf 3.2.2 CONFIG QUIET)
if(NOT SDL3_ttf_FOUND)
    if (WIN32)
        FetchContent_Declare(
            SDL3_ttf
            URL https://www.libsdl.org/projects/SDL_ttf/release/SDL3_ttf-devel-3.2.2-VC.zip
            DOWNLOAD_NO_PROGRESS ON
        )
        FetchContent_MakeAvailable(SDL3_ttf)
        list(PREPEND CMAKE_PREFIX_PATH "${sdl3_ttf_SOURCE_DIR}")
        find_package(SDL3_ttf CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            SDL3_ttf
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
            GIT_TAG release-3.2.2
            GIT_SHALLOW TRUE
        )

        set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
        set(SDLTTF_HARFBUZZ ON CACHE BOOL "" FORCE)
        set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
        set(SDLTTF_INSTALL OFF CACHE BOOL "" FORCE)

        # Emscripten-specific settings for SDL3_ttf
        if(EMSCRIPTEN)
            list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/emscripten")
        endif()

        FetchContent_MakeAvailable(SDL3_ttf)
    endif()
endif()

# SDL3_mixer
find_package(SDL3_mixer CONFIG QUIET)
if(NOT SDL3_mixer_FOUND)
    if(WIN32)
        FetchContent_Declare(
            SDL3_mixer
            URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-3.2.0/SDL3_mixer-devel-3.2.0-VC.zip
            DOWNLOAD_NO_PROGRESS ON
        )
        FetchContent_MakeAvailable(SDL3_mixer)
        list(PREPEND CMAKE_PREFIX_PATH "${sdl3_mixer_SOURCE_DIR}")
        find_package(SDL3_mixer CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            SDL3_mixer
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
            GIT_TAG release-3.2.0
            GIT_SHALLOW TRUE
        )

        set(SDLMIXER_VENDORED OFF CACHE BOOL "" FORCE)
        set(SDLMIXER_WAV     ON  CACHE BOOL "" FORCE)
        set(SDLMIXER_OGG     ON  CACHE BOOL "" FORCE)
        set(SDLMIXER_MP3     ON  CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(SDL3_mixer)
    endif()
endif()

# VLD (Windows only)
if (WIN32)
    find_package(VLD CONFIG)
endif()

# --------------------- Public interface ---------------------
target_include_directories(Minigin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${glm_SOURCE_DIR}
    ${sdl3_SOURCE_DIR}/include
    ${sdl3_ttf_SOURCE_DIR}/include
    ${VLD_INCLUDE_DIR}
)

target_link_libraries(Minigin PUBLIC
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    ${VLD_LIBRARIES}
)

target_include_directories(Minigin PUBLIC ${sdl3_mixer_SOURCE_DIR}/include)
target_link_libraries(Minigin PUBLIC SDL3_mixer::SDL3_mixer)

if(WIN32)
    target_link_libraries(Minigin PUBLIC xinput)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(Minigin PRIVATE /W4 /WX)
else()
    target_compile_options(Minigin PRIVATE -Wall -Wextra -Werror)
endif()