// bounding box
#pragma once

#include "gl_def.h"

namespace BoundingBox
{

//
class Base
{
public:
  virtual ~Base() = default;
  //
  virtual bool             check(double x, double y) const = 0;
  virtual Graphics::Locate getLocate() const               = 0;
  virtual Graphics::Locate getBottom() const { return getLocate(); }
  virtual double           getRadius() const { return 0.0; }
};

//
class Rect : public Base
{
  double left      = 0.0;
  double right     = 0.0;
  double top       = 0.0;
  double bottom    = 0.0;
  bool   on_cursor = false;

public:
  enum class Align : int
  {
    LeftTop,
    RightTop,
    CenterTop,
    LeftCenter,
    RightCenter,
    Center,
    LeftBottom,
    RightBottom,
    CenterBottom,
  };

  Rect() {}
  Rect(double x, double y, double w, double h, Align al = Align::LeftTop)
  {
    switch (al)
    {
    case Align::LeftTop:
    case Align::LeftCenter:
    case Align::LeftBottom:
      left  = x;
      right = x + w;
      break;
    case Align::RightTop:
    case Align::RightCenter:
    case Align::RightBottom:
      left  = x - w;
      right = x;
      break;
    case Align::CenterTop:
    case Align::Center:
    case Align::CenterBottom:
      left  = x - w * 0.5;
      right = x + w * 0.5;
      break;
    }
    switch (al)
    {
    case Align::LeftTop:
    case Align::RightTop:
    case Align::CenterTop:
      top    = y;
      bottom = y + h;
      break;
    case Align::LeftCenter:
    case Align::RightCenter:
    case Align::Center:
      top    = y - h;
      bottom = y;
      break;
    case Align::LeftBottom:
    case Align::RightBottom:
    case Align::CenterBottom:
      top    = y - h * 0.5;
      bottom = y + h * 0.5;
      break;
    }
  }
  ~Rect() = default;

  bool check(double x, double y) const override
  {
    if (left < x && right > x && top < y && bottom > y)
      return true;
    return false;
  }

  Graphics::Locate getLocate() const override
  {
    Graphics::Locate l;
    l.x = left;
    l.y = top;
    return l;
  }

  Graphics::Locate getBottom() const override
  {
    Graphics::Locate l;
    l.x = right;
    l.y = bottom;
    return l;
  }
};

}; // namespace BoundingBox
