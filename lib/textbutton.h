#pragma once

#include "font.h"
#include "parts.h"
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

using ID = Parts::IDPtr;

//
void initialize(FontDraw::WidgetPtr font);
//
ID setButton(std::string caption, double x, double y, PressCallback cb,
             bool catch_enter = false);
//
void setColor(ColorType ct, float r, float g, float b, float a = 1.0f);
//
void bindLayer(std::string layer = DefaultLayer);
//
void clearLayer(std::string layer = DefaultLayer);
//
void erase(ID);
//
void update();
} // namespace TextButton
