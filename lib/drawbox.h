#pragma once

#include "font.h"
#include <memory>

namespace DrawBox
{

//
struct Box
{
  virtual ~Box()                             = default;
  virtual double getBaseX() const            = 0;
  virtual double getBaseY() const            = 0;
  virtual double getWidth() const            = 0;
  virtual double getHeight() const           = 0;
  virtual void   begin()                     = 0;
  virtual void   end()                       = 0;
  virtual void   setDrawSize(double, double) = 0;
  virtual void   setDragScroll(bool)         = 0;
};
using BoxPtr = std::shared_ptr<Box>;

//
void initialize();

//
BoxPtr create(FontDraw::WidgetPtr f, int x, int y, int w, int h);

} // namespace DrawBox
