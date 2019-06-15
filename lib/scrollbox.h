#pragma once

#include "gl_def.h"
#include "parts.h"

namespace ScrollBox
{
//
struct Base : public Parts::ID
{
  virtual void set(double x, double y, double w, double h) = 0;
  virtual void append(Parts::IDPtr)                        = 0;
  virtual void erase(Parts::IDPtr)                         = 0;
  virtual void clear()                                     = 0;
  virtual void drawSheet(bool s, Graphics::Color scol)     = 0;
  virtual void setDepth(float d)                           = 0;
};

std::shared_ptr<Base> create();

//
void update();

} // namespace ScrollBox
