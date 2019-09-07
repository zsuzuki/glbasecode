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

// 描画エリア(シザリング)指定
struct DrawArea
{
  double x = 0.0;
  double y = 0.0;
  double w = 0.0;
  double h = 0.0;
  bool   e = false;

  inline void set(const DrawArea& old) const;
};

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
void        setWindowSize(WindowSize);
WindowSize  getWindowSize();
Locate      getMousePosition();
void        setClipboardString(const char*);
const char* getClipboardString();
void        switchFullScreen();
void        enableScissor(double x, double y, double w, double h);
void        disableScissor();
KeyInput&   getKeyInput();
void        enableEvent();
void        disableEvent(OffEventCallback);
bool        isEnabledEvent();
void        openPulldown();
void        closePulldown();
Locate      getPulldownCursor();
Vector      getScroll();

//
void
DrawArea::set(const DrawArea& old) const
{
  if (e)
  {
    if (old.e == false || x != old.x || y != old.y || w != old.w || h != old.h)
      Graphics::enableScissor(x, y, w, h);
  }
  else if (old.e)
    Graphics::disableScissor();
}

} // namespace Graphics
