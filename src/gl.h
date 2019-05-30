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
using KeyCallback      = void (*)(int, int, int, int);
using DropCallback     = void (*)(int, const char**);
using MouseBtnCallback = void (*)(int, int, int);
using TextCallback     = void (*)(int);

struct WindowSize
{
  double width;
  double height;
};
struct Locate
{
  double x, y;
};
//
bool        initialize(const char* appname, int w, int h);
GLFWwindow* setupFrame();
void        cleanupFrame();
void        terminate();
void        finish();
Locate      calcLocate(double x, double y, bool asp = false);
void        setKeyCallback(KeyCallback);
void        setDropCallback(DropCallback);
void        setMouseButtonCallback(MouseBtnCallback);
void        setTextInputCallback(KeyCallback, TextCallback); // for text only
void        setTextButtonCallback(MouseBtnCallback); // for textbutton only
WindowSize  getWindowSize();
Locate      getMousePosition();
} // namespace Graphics
