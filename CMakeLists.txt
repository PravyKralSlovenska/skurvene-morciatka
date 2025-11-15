cmake_minimum_required(VERSION 3.10)
project(morciatko)

cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# MAIN EXECUTABLE
file(GLOB MAIN_SOURCES
    src/*/*/*.cpp
    src/*/*.cpp
    src/*.cpp 
    src/*/*/*.c
    src/*/*.c
    src/*.c
)

list(FILTER MAIN_SOURCES EXCLUDE REGEX ".*main\\.cpp$") # odstrani main z MAIN_SOURCES

add_executable(morciatko 
    ${MAIN_SOURCES}
    src/main.cpp
)

target_link_libraries(morciatko
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${FREETYPE2_LIBRARIES}
    ${OPENAL_LIBRARIES}
    dl
)

# TESTING SETUP
option(BUILD_TESTS "Build the tests" ON)

if(BUILD_TESTS)
    enable_testing()
    
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
    
    add_library(morciatko_lib STATIC ${MAIN_SOURCES})
    target_link_libraries(morciatko_lib
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${FREETYPE2_LIBRARIES}
        ${OPENAL_LIBRARIES}
        dl
    )
    
    file(GLOB TEST_SOURCES 
        tests/*.cpp 
        tests/*/*.cpp
    )
    
    add_executable(morciatko_tests ${TEST_SOURCES})

    target_link_libraries(morciatko_tests
        morciatko_lib
        GTest::GTest
        GTest::Main
        pthread
    )
    
    include(GoogleTest)
    gtest_discover_tests(morciatko_tests)
    
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS morciatko_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
