cmake_minimum_required(VERSION 3.7)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(gltest)

find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(include)
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})

set(main_src src/main.cpp src/font.cpp src/test.cpp src/primitive2d.cpp)

add_executable(${PROJECT_NAME} ${main_src})

find_library(OpenGL_LIBRARY OpenGL)
target_link_directories(${PROJECT_NAME} PRIVATE /usr/local/lib)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${GLFW_LIBRARIES}
    ${OpenGL_LIBRARY}
    ${FREETYPE_LIBRARY}
    )
