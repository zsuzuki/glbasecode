cmake_minimum_required(VERSION 3.11)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(gltest)

find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
if (WIN32)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
else()
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(Boost 1.65.0 REQUIRED COMPONENTS iostreams)
endif()

include_directories(lib)
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

set(main_src
    src/main.cpp
    lib/gl.cpp
    lib/font.cpp
    lib/primitive2d.cpp
    lib/text.cpp
    lib/textbox.cpp
    lib/textbutton.cpp
    lib/codeconv.cpp
    lib/exec.cpp)

add_executable(${PROJECT_NAME} ${main_src})

if (WIN32)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    opengl32
    glfw
    ${FREETYPE_LIBRARY}
    ${GLEW_LIBRARIES}
    )
else()
find_library(OpenGL_LIBRARY OpenGL)
target_link_directories(${PROJECT_NAME} PRIVATE /usr/local/lib)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${Boost_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${OpenGL_LIBRARY}
    ${FREETYPE_LIBRARY}
    )
endif()
