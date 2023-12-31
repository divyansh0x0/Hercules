cmake_minimum_required(VERSION 3.8)

project("Hercules")

add_subdirectory("Engine")
add_subdirectory("UI")
set_target_properties(Engine PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(UI PROPERTIES  LINKER_LANGUAGE CXX)

include_directories(Engine)
include_directories(UI)
add_subdirectory("Launcher")