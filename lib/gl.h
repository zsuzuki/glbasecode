#pragma once

#if defined(_MSC_VER) // Windows
#define _USE_MATH_DEFINES
#include <Windows.h>
//
#include <GL/glew.h>
#else
#include <OpenGL/gl.h>
#endif
#include "gl_def.h"
#include <GLFW/glfw3.h>

namespace Graphics
{
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
void        setClickCallback(ClickCallback);
void        setScrollCallback(ScrollCallback);
WindowSize  getWindowSize();
Locate      getMousePosition();
} // namespace Graphics
