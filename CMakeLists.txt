cmake_minimum_required(VERSION 3.10)
project(morciatko)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
pkg_check_modules(FREETYPE2 REQUIRED freetype2)
pkg_check_modules(OPENAL REQUIRED openal)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
    ${FREETYPE2_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
)

file(GLOB SOURCES
    src/*.cpp 
    src/*.c
    src/*/*.cpp
    src/*/*.c
    main.cpp
)

add_executable(morciatko ${SOURCES})

target_link_libraries(morciatko
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${FREETYPE2_LIBRARIES}
    ${OPENAL_LIBRARIES}
    dl
)
