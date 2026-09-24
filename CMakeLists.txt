cmake_minimum_required(VERSION 3.10)
project(minimal-3Dmodeler VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Platform-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# Find Raylib
find_path(RAYLIB_INCLUDE_DIR raylib.h
    PATHS C:/w64devkit/include
    REQUIRED)

find_library(RAYLIB_LIBRARY NAMES raylib
    PATHS C:/w64devkit/lib
    REQUIRED)

# Build executable
file(GLOB_RECURSE SOURCES "src/*.c" "core/*.c" "renderer/*.c")
file(GLOB_RECURSE HEADERS "src/*.h" "core/*.h" "renderer/*.h")

add_executable(minimal-3dmodeler ${SOURCES} ${HEADERS})
target_include_directories(minimal-3dmodeler PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${RAYLIB_INCLUDE_DIR})

target_link_libraries(minimal-3dmodeler PRIVATE ${RAYLIB_LIBRARY})

# Platform-specific libraries
if(WIN32)
    target_link_libraries(minimal-3dmodeler PRIVATE opengl32 gdi32 winmm)
endif()

# Copy shader files
file(GLOB_RECURSE SHADERS "*.fs" "*.vs" "*.gs")
foreach(shader ${SHADERS})
    get_filename_component(shader_name ${shader} NAME)
    configure_file(${shader} ${CMAKE_CURRENT_BINARY_DIR}/${shader_name} COPYONLY)
endforeach()
