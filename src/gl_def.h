#pragma once

namespace Graphics
{
//
using KeyCallback      = void (*)(int, int, int, int);
using DropCallback     = void (*)(int, const char**);
using MouseBtnCallback = void (*)(int, int, int);
using TextCallback     = void (*)(int);
using TextBtnCallback  = void (*)(int, bool);

struct WindowSize
{
  double width;
  double height;
};
struct Locate
{
  double x, y;
};
struct Color
{
  float r;
  float g;
  float b;
  float a;

  Color(float rr = 1.0f, float gg = 1.0f, float bb = 1.0f, float aa = 1.0f)
  {
    set(rr, gg, bb, aa);
  }
  void set(float rr, float gg, float bb, float aa = 1.0f)
  {
    r = rr;
    g = gg;
    b = bb;
    a = aa;
  }
};
} // namespace Graphics
