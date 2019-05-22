// font render utility
#pragma once

#include <memory>

struct GLFWwindow;

namespace FontDraw
{
// フォント1つ分の管理
class Widget
{
public:
  virtual ~Widget() = default;

  virtual void setSize(float w, float h)                           = 0;
  virtual void setColor(float r, float g, float b, float a = 1.0f) = 0;
  virtual void print(const char* msg, float x, float y)            = 0;
};
std::shared_ptr<Widget> create(const char* fontname);

//
bool initialize();
void render(GLFWwindow*);
void terminate();

} // namespace FontDraw
