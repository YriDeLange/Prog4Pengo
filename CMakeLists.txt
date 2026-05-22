cmake_minimum_required(VERSION 3.26)
project(Prog4Pengo)

# ====================== ENGINE (static library) ======================
add_subdirectory(Minigin)

# ====================== GAME (executable) ======================
add_subdirectory(Game)