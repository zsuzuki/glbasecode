#pragma once

#if defined(_MSC_VER) // Windows
#define _USE_MATH_DEFINES
#include <Windows.h>
//
#include <GL/glew.h>
#else
#include <OpenGL/gl.h>
#endif
#include <GLFW/glfw3.h>

namespace Graphics
{
//
using KeyCallback  = void (*)(int, int, int, int);
using DropCallback = void (*)(int, const char**);

struct WindowSize
{
  double width;
  double height;
};
//
bool        initialize(const char* appname, int w, int h);
GLFWwindow* setupFrame();
void        cleanupFrame();
void        terminate();
void        finish();
void        setKeyCallback(KeyCallback);
void        setDropCallback(DropCallback);
WindowSize  getWindowSize();
} // namespace Graphics
