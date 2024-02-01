cmake_minimum_required(VERSION 3.12)
project(fractals)

set(CMAKE_CXX_STANDARD 11)
set(VE_DEPENDENCIES_LOCATION dependencies)
set(VE_SOURCE_LOCATION src)

file(GLOB_RECURSE VE_SOURCE_FILES CONFIGURE_DEPENDS "${VE_SOURCE_LOCATION}/*.cpp")
file(GLOB_RECURSE VE_HEADER_FILES CONFIGURE_DEPENDS "${VE_SOURCE_LOCATION}/*.h")
file(GLOB_RECURSE VE_SHADER_FILES CONFIGURE_DEPENDS "$shaders/*.*")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(${PROJECT_NAME} ${VE_SOURCE_FILES} ${VE_HEADER_FILES} ${VE_SHADER_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VE_SOURCE_FILES} ${VE_HEADER_FILES} $(VE_SHADER_FILES))

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:preprocessor /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# raylib
add_subdirectory(${VE_DEPENDENCIES_LOCATION}/raylib)
target_link_libraries(${PROJECT_NAME} raylib)
