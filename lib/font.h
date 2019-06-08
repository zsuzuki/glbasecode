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

  virtual void setSize(float w, float h)                = 0;
  virtual void setColor(const Graphics::Color)          = 0;
  virtual void print(const char* msg, float x, float y) = 0;
};
using WidgetPtr = std::shared_ptr<Widget>;
WidgetPtr create(const char* fontname);

//
bool initialize();
void render(GLFWwindow*);
void terminate();

} // namespace FontDraw