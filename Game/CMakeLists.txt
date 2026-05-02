cmake_minimum_required(VERSION 3.26)
project(PengoGame)

if(EMSCRIPTEN)
    set(TARGET_NAME minigin_web)
    set(TARGET_PARAM )
else()
    set(TARGET_NAME PengoGame)
    set(TARGET_PARAM WIN32)
endif()

add_executable(${TARGET_NAME} ${TARGET_PARAM}
    Main.cpp
)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)

target_link_libraries(${TARGET_NAME} PRIVATE Minigin)


add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Data"
        "$<TARGET_FILE_DIR:${TARGET_NAME}>/Data"
    COMMENT "Copying Data folder"
)

if(NOT EMSCRIPTEN AND WIN32)
    find_package(SDL3 CONFIG REQUIRED)
    find_package(SDL3_ttf CONFIG REQUIRED)
    find_package(SDL3_mixer CONFIG REQUIRED)

    # Copy SDL3.dll
    if(TARGET SDL3::SDL3)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:SDL3::SDL3>"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>"
            COMMENT "Copying SDL3.dll"
            VERBATIM
        )
    endif()

    # Copy SDL3_ttf.dll + SDL3_mixer.dll
    foreach(dllTarget SDL3_ttf::SDL3_ttf SDL3_mixer::SDL3_mixer)
        if(TARGET ${dllTarget})
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${dllTarget}>"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                COMMENT "Copying SDL dll"
                VERBATIM
            )
        endif()
    endforeach()

    # Steam support (if enabled)
    if(Steamworks_FOUND)
        if(WIN32 AND DEFINED Steamworks_RUNTIME_DLL)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${Steamworks_RUNTIME_DLL}"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                COMMENT "Copying steam_api64.dll"
            )
        endif()

        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "480" > "$<TARGET_FILE_DIR:${TARGET_NAME}>/steam_appid.txt"
            COMMENT "Creating steam_appid.txt"
        )
    endif()

    # Visual Studio settings
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
    set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET_NAME}>")
endif()

# ====================== EMSCRIPTEN SETTINGS ======================
if(EMSCRIPTEN)
    set(TARGET_NAME minigin_web)

    # Emscripten link flags
    set(EMSCRIPTEN_LINK_FLAGS 
        "-sUSE_SDL=3 \
         -sUSE_SDL_TTF=3 \
         -sUSE_FREETYPE=1 \
         -sUSE_HARFBUZZ=1 \
         -sALLOW_MEMORY_GROWTH=1 \
         -sINITIAL_MEMORY=64MB \
         --preload-file \"${CMAKE_SOURCE_DIR}/Data@/Data\" \
         --shell-file \"${CMAKE_SOURCE_DIR}/web/index.html\"")

    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
        COMPILE_FLAGS "-g0"
        LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS}"
    )

    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()