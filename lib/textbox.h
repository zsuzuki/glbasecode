#pragma once

#include "font.h"
#include "gl_def.h"
#include "parts.h"
#include "pulldown.h"
#include "text_def.h"
#include <memory>
#include <string>

namespace TextBox
{
using InputStyle = Text::InputStyle;

//
struct Item : public Parts::ID
{
  virtual void        setPlaceHolder(std::string)          = 0;
  virtual void        drawBorder(bool draw)                = 0;
  virtual std::string getText() const                      = 0;
  virtual void        setFontColor(Graphics::Color)        = 0;
  virtual void        setBGColor(Graphics::Color)          = 0;
  virtual void        setPlaceHolderColor(Graphics::Color) = 0;
  virtual void        setMaxLength(size_t)                 = 0;
  virtual void        setPulldown(Pulldown::ID)            = 0;
  virtual void        setInputStyle(InputStyle)            = 0;
  virtual void        setSlider(Parts::IDPtr, int = 3)     = 0;
};
using ItemPtr = std::shared_ptr<Item>;

//
void initialize(FontDraw::WidgetPtr font);
//
void bindLayer(std::string layer = Parts::DefaultLayer);
//
void clearLayer(std::string layer = Parts::DefaultLayer);
//
void eraseItem(ItemPtr);
//
ItemPtr create(std::string, double x, double y, double w = 0.0, double h = 0.0);
//
void update();
} // namespace TextBox
