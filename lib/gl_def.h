#pragma once

namespace Graphics
{
using KeyCallback    = void (*)(int, int, int, int);
using TextCallback   = void (*)(int);
using ScrollCallback = void (*)(double, double);

// クリックされた時に呼び出す
struct ClickCallback
{
  enum class Action : int
  {
    Press,
    Release
  };
  using Func     = void (*)(Action, bool);
  Func func      = nullptr;
  bool use_enter = false;
};

// イベントが発行されないときの復帰用イベント
struct OffEventCallback
{
  enum class Action : int
  {
    Click,
    Release,
    EnterKey,
    EscapeKey
  };
  using Func = bool (*)(Action);
  Func func  = nullptr;
};

//
struct WindowSize
{
  double width;
  double height;
};
struct Locate
{
  double x, y;
};
struct Vector
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
static const Color White{1.0f, 1.0f, 1.0f, 1.0f};
static const Color Black{0.0f, 0.0f, 0.0f, 1.0f};
static const Color Red{1.0f, 0.0f, 0.0f, 1.0f};
static const Color Green{0.0f, 1.0f, 0.0f, 1.0f};
static const Color Blue{0.0f, 0.0f, 1.0f, 1.0f};
static const Color Navy{0.0f, 0.0f, 0.5f, 1.0f};
static const Color Gray{0.5f, 0.5f, 0.5f, 1.0f};
static const Color DarkGray{0.3f, 0.3f, 0.3f, 1.0f};
static const Color LightGray{0.7f, 0.7f, 0.7f, 1.0f};
static const Color Yellow{1.0f, 1.0f, 0.0f, 1.0f};
static const Color Magenta{1.0f, 0.0f, 1.0f, 1.0f};
static const Color Cyan{0.0f, 1.0f, 1.0f, 1.0f};
static const Color Orange{1.0f, 0.6f, 0.0f, 1.0f};
static const Color Sepia{0.45f, 0.32f, 0.14f, 1.0f};
static const Color ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

} // namespace Graphics
