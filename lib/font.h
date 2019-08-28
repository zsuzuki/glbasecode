// font render utility
#pragma once

#include "gl_def.h"
#include <memory>

struct GLFWwindow;

namespace FontDraw
{
// フォント1つ分の管理
class Widget
{
public:
  virtual ~Widget() = default;

  virtual void  setSize(float w, float h)                           = 0;
  virtual void  setColor(const Graphics::Color)                     = 0;
  virtual void  print(const char* msg, float x, float y)            = 0;
  virtual void  setDepth(float d)                                   = 0;
  virtual void  pushDepth(float d)                                  = 0;
  virtual void  popDepth()                                          = 0;
  virtual void  setDrawArea(double x, double y, double w, double h) = 0;
  virtual void  clearDrawArea()                                     = 0;
  virtual float getSizeX() const                                    = 0;
  virtual float getSizeY() const                                    = 0;
};
using WidgetPtr = std::shared_ptr<Widget>;
WidgetPtr create(const char* fontname);

//
bool initialize();
void render(GLFWwindow*);
void terminate();

} // namespace FontDraw
