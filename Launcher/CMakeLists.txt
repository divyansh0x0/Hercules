# CMakeList.txt : CMake project for Engine, include source and define
# project specific logic here.
#

# Add source to this project's executable.
cmake_minimum_required(VERSION 3.8)


#--------------------------------------------------------------------
# Hide the console window in visual studio projects - Release
#--------------------------------------------------------------------
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
#link_libraries(Engine)
#link_libraries(UI)
add_executable(Launcher "Launcher.cpp" "Launcher.h")
target_include_directories(Launcher PRIVATE Engine)
target_include_directories(Launcher PRIVATE UI)
