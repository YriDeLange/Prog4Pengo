cmake_minimum_required(VERSION 3.26)
project(Prog4Pengo)

if(EMSCRIPTEN)
    add_compile_options(-pthread)
    add_link_options(-sPTHREAD_POOL_SIZE=4)
endif()

# ====================== ENGINE (static library) ======================
add_subdirectory(Minigin)

# ====================== GAME (executable) ======================
add_subdirectory(Game)