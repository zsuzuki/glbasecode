#pragma once

#include "font.h"
#include "gl_def.h"
#include "parts.h"
#include <string>

namespace Label
{
//
struct Base : public Parts::ID
{
  virtual void setText(std::string)          = 0;
  virtual void setFontColor(Graphics::Color) = 0;
  virtual void setBGColor(Graphics::Color)   = 0;
};

using ID    = std::shared_ptr<Base>;
using Color = Graphics::Color;

//
void initialize(FontDraw::WidgetPtr font);
//
ID create(std::string str, double x, double y, Color fg, Color bg);
//
void bindLayer(std::string layer = Parts::DefaultLayer);
//
void clearLayer(std::string layer = Parts::DefaultLayer);
//
void erase(ID);
//
void update();

} // namespace Label
