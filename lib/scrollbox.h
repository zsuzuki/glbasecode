#pragma once

#include "gl_def.h"
#include "parts.h"
#include <string>

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
  virtual void setScrollConstraint(bool sx, bool sy)       = 0;
  virtual void setSticky(bool, bool)                       = 0;
  virtual void setFocusBorderColor(Graphics::Color)        = 0;
};

using SBoxPtr = std::shared_ptr<Base>;

//
void bindLayer(std::string layer = Parts::DefaultLayer);
//
void clearLayer(std::string layer = Parts::DefaultLayer);
//
void erase(SBoxPtr);

//
SBoxPtr create();

//
void update();

} // namespace ScrollBox
