#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#if defined(_MSC_VER) // Windows
#define _USE_MATH_DEFINES
#include <Windows.h>
//
#include <GL/glew.h>
#else
#include <OpenGL/gl.h>
#endif
#include "gl_def.h"
#include "key.h"
#include <GLFW/glfw3.h>

namespace Graphics
{
using KeyInput = const Key::Input;
//
bool        initialize(const char* appname, int w, int h);
GLFWwindow* setupFrame();
void        cleanupFrame();
void        terminate();
void        finish();
Locate      calcLocate(double x, double y, bool asp = false);
void        setDropCallback(DropCallback);
void        setTextInputCallback(KeyCallback, TextCallback);
void        setClickCallback(ClickCallback);
void        setScrollBoxFunction(ScrollCallback, KeyCallback);
WindowSize  getWindowSize();
Locate      getMousePosition();
void        setClipboardString(const char*);
const char* getClipboardString();
void        switchFullScreen();
void        enableScissor(double x, double y, double w, double h);
void        disableScissor();
KeyInput&   getKeyInput();
} // namespace Graphics
