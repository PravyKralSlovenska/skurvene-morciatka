cmake_minimum_required(VERSION 3.16)
project(morciatko LANGUAGES C CXX)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0)
    # Some bundled third-party projects still declare very old policy ranges.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
endif()

cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

set(DEFAULT_USE_BUNDLED_DEPS OFF)
if(WIN32)
    set(DEFAULT_USE_BUNDLED_DEPS ON)
endif()
option(USE_BUNDLED_DEPS "Use bundled third-party libraries from include/" ${DEFAULT_USE_BUNDLED_DEPS})

set(MORCIATKO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(MORCIATKO_IMGUI_DIR ${CMAKE_SOURCE_DIR}/include/imgui)
set(MORCIATKO_IMGUI_BACKENDS_DIR ${CMAKE_SOURCE_DIR}/include/imgui/backends)

find_path(GLM_INCLUDE_DIR glm/glm.hpp
    HINTS
        ${CMAKE_SOURCE_DIR}/include/glm
)

if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "GLM headers were not found. Expected glm/glm.hpp in include/glm or system paths.")
endif()

if(TARGET OpenGL::GL)
    set(MORCIATKO_OPENGL_TARGET OpenGL::GL)
elseif(TARGET OpenGL::OpenGL)
    set(MORCIATKO_OPENGL_TARGET OpenGL::OpenGL)
else()
    set(MORCIATKO_OPENGL_TARGET ${OPENGL_LIBRARIES})
endif()

if(USE_BUNDLED_DEPS)
    message(STATUS "Using bundled dependencies from include/")

    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/include/glfw-3.4 EXCLUDE_FROM_ALL)

    add_subdirectory(${CMAKE_SOURCE_DIR}/include/freetype-2.10.0 EXCLUDE_FROM_ALL)

    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/include/openal-soft EXCLUDE_FROM_ALL)

    set(MORCIATKO_GLFW_TARGET glfw)
    set(MORCIATKO_FREETYPE_TARGET freetype)
    set(MORCIATKO_OPENAL_TARGET OpenAL::OpenAL)
else()
    message(STATUS "Using system dependencies")

    find_package(PkgConfig REQUIRED)

    find_package(glfw3 QUIET)
    if(TARGET glfw)
        set(MORCIATKO_GLFW_TARGET glfw)
    elseif(TARGET glfw3)
        set(MORCIATKO_GLFW_TARGET glfw3)
    else()
        pkg_search_module(GLFW REQUIRED glfw3)
        add_library(morciatko_glfw INTERFACE)
        target_include_directories(morciatko_glfw INTERFACE ${GLFW_INCLUDE_DIRS})
        target_link_libraries(morciatko_glfw INTERFACE ${GLFW_LIBRARIES})
        set(MORCIATKO_GLFW_TARGET morciatko_glfw)
    endif()

    find_package(Freetype QUIET)
    if(TARGET Freetype::Freetype)
        set(MORCIATKO_FREETYPE_TARGET Freetype::Freetype)
    elseif(TARGET freetype)
        set(MORCIATKO_FREETYPE_TARGET freetype)
    else()
        pkg_check_modules(FREETYPE2 REQUIRED freetype2)
        add_library(morciatko_freetype INTERFACE)
        target_include_directories(morciatko_freetype INTERFACE ${FREETYPE2_INCLUDE_DIRS})
        target_link_libraries(morciatko_freetype INTERFACE ${FREETYPE2_LIBRARIES})
        set(MORCIATKO_FREETYPE_TARGET morciatko_freetype)
    endif()

    find_package(OpenAL QUIET)
    if(TARGET OpenAL::OpenAL)
        set(MORCIATKO_OPENAL_TARGET OpenAL::OpenAL)
    else()
        pkg_check_modules(OPENAL REQUIRED openal)
        add_library(morciatko_openal INTERFACE)
        target_include_directories(morciatko_openal INTERFACE ${OPENAL_INCLUDE_DIRS})
        target_link_libraries(morciatko_openal INTERFACE ${OPENAL_LIBRARIES})
        set(MORCIATKO_OPENAL_TARGET morciatko_openal)
    endif()
endif()

# ImGui sources
file(GLOB IMGUI_SOURCES
    include/imgui/*.cpp
    include/imgui/backends/imgui_impl_glfw.cpp
    include/imgui/backends/imgui_impl_opengl3.cpp
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

function(configure_morciatko_target target_name)
    target_include_directories(${target_name}
        PRIVATE
            ${MORCIATKO_INCLUDE_DIR}
            ${MORCIATKO_IMGUI_DIR}
            ${MORCIATKO_IMGUI_BACKENDS_DIR}
            ${GLM_INCLUDE_DIR}
    )

    target_link_libraries(${target_name}
        PRIVATE
            ${MORCIATKO_OPENGL_TARGET}
            ${MORCIATKO_GLFW_TARGET}
            ${MORCIATKO_FREETYPE_TARGET}
            ${MORCIATKO_OPENAL_TARGET}
            Threads::Threads
            ${CMAKE_DL_LIBS}
    )
endfunction()

add_executable(morciatko 
    ${MAIN_SOURCES}
    ${IMGUI_SOURCES}
    src/main.cpp
)

configure_morciatko_target(morciatko)

if(WIN32 AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
    add_custom_command(TARGET morciatko POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:morciatko>
            $<TARGET_FILE_DIR:morciatko>
        COMMAND_EXPAND_LISTS
    )
endif()

# TESTING SETUP
option(BUILD_TESTS "Build the tests" ON)

if(BUILD_TESTS)
    enable_testing()

    if(USE_BUNDLED_DEPS)
        set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        add_subdirectory(${CMAKE_SOURCE_DIR}/include/googletest EXCLUDE_FROM_ALL)

        set(MORCIATKO_GTEST_TARGET GTest::gtest)
        set(MORCIATKO_GTEST_MAIN_TARGET GTest::gtest_main)
    else()
        find_package(GTest REQUIRED)

        if(TARGET GTest::gtest)
            set(MORCIATKO_GTEST_TARGET GTest::gtest)
            set(MORCIATKO_GTEST_MAIN_TARGET GTest::gtest_main)
        else()
            set(MORCIATKO_GTEST_TARGET GTest::GTest)
            set(MORCIATKO_GTEST_MAIN_TARGET GTest::Main)
        endif()
    endif()
    
    add_library(morciatko_lib STATIC ${MAIN_SOURCES} ${IMGUI_SOURCES})
    configure_morciatko_target(morciatko_lib)
    
    file(GLOB TEST_SOURCES 
        tests/*.cpp 
        tests/*/*.cpp
    )
    
    add_executable(morciatko_tests ${TEST_SOURCES})

    target_include_directories(morciatko_tests
        PRIVATE
            ${MORCIATKO_INCLUDE_DIR}
            ${MORCIATKO_IMGUI_DIR}
            ${MORCIATKO_IMGUI_BACKENDS_DIR}
            ${GLM_INCLUDE_DIR}
    )

    target_link_libraries(morciatko_tests
        morciatko_lib
        ${MORCIATKO_GTEST_TARGET}
        ${MORCIATKO_GTEST_MAIN_TARGET}
        Threads::Threads
    )
    
    include(GoogleTest)
    gtest_discover_tests(morciatko_tests
        PROPERTIES
            ENVIRONMENT "MORCIATKO_WORLD_SEED=1"
    )
    
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS morciatko_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
