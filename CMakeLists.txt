cmake_minimum_required(VERSION 3.10)
project(morciatko)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
)

file(GLOB SOURCES 
    src/*.cpp 
    src/*.c
    main.cpp
)

add_executable(morciatko ${SOURCES})

target_link_libraries(morciatko
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    dl
)
