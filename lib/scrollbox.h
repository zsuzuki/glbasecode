#pragma once

#include "parts.h"

namespace ScrollBox
{
//
struct Base
{
  virtual void set(double x, double y, double w, double h) = 0;
  virtual void append(Parts::IDPtr)                        = 0;
  virtual void erase(Parts::IDPtr)                         = 0;
  virtual void clear()                                     = 0;
};

std::shared_ptr<Base> create();

//
void update();

} // namespace ScrollBox
