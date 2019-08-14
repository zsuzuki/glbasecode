#pragma once

#include "font.h"
#include "parts.h"
#include "pulldown.h"
#include <functional>
#include <memory>
#include <string>

namespace TextButton
{
//
using PressCallback = std::function<void()>;
enum class ColorType : int
{
  UnFocusBG,
  FocusBG,
  PressBG,
  UnFocusFont,
  FocusFont,
  PressFont,
};
static constexpr const char* DefaultLayer = "default";

//
struct Base : public Parts::ID
{
  virtual void setColor(ColorType ct, Graphics::Color c) = 0;
  virtual void setPulldown(Pulldown::ID pd)              = 0;
};

using ID = std::shared_ptr<Base>;

//
void initialize(FontDraw::WidgetPtr font);
//
ID setButton(std::string caption, double x, double y, PressCallback cb,
             bool catch_enter = false);
//
void setDefaultColor(ColorType ct, Graphics::Color c);
//
void bindLayer(std::string layer = DefaultLayer);
//
void clearLayer(std::string layer = DefaultLayer);
//
void erase(ID);
//
void update();
} // namespace TextButton
