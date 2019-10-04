#pragma once

#include "font.h"
#include "gl_def.h"
#include "parts.h"
#include <functional>
#include <string>

namespace CheckBox
{
// 値変更コールバック
using Changed = std::function<void(bool)>;

//
struct Base : public Parts::ID
{
  virtual void setText(std::string)         = 0;
  virtual void setOffText(std::string)      = 0;
  virtual void setValue(bool)               = 0;
  virtual bool getValue() const             = 0;
  virtual void setOnColor(Graphics::Color)  = 0;
  virtual void setOffColor(Graphics::Color) = 0;
  virtual void setChanged(Changed)          = 0;

  operator bool() const { return getValue(); }
};

using ID = std::shared_ptr<Base>;

//
void initialize(FontDraw::WidgetPtr font);
//
ID create(std::string on_str, double x, double y, bool sw);
//
void bindLayer(std::string layer = Parts::DefaultLayer);
//
void clearLayer(std::string layer = Parts::DefaultLayer);
//
void erase(ID);
//
void update();

} // namespace CheckBox
