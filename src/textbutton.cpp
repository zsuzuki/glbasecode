#include "textbutton.h"
#include "font.h"
#include "gl.h"
#include "primitive2d.h"
#include <iostream>
#include <list>
#include <map>
#include <memory>

namespace TextButton
{
namespace
{

struct Color
{
  float r, g, b, a;
};
constexpr Color white    = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr Color darkgray = {0.3f, 0.3f, 0.3f, 1.0f};
constexpr Color gray     = {0.5f, 0.5f, 0.5f, 1.0f};

std::map<ColorType, Color> color_map = {
    {ColorType::UnFocusBG, darkgray}, {ColorType::FocusBG, gray},
    {ColorType::PressBG, white},      {ColorType::UnFocusFont, white},
    {ColorType::FocusFont, white},    {ColorType::PressFont, darkgray},
};

//
struct Button
{

  std::string   caption;
  double        lx, rx, ty, by;
  double        x, y;
  PressCallback cb;
  bool          catch_enter;
  bool          press;
};
using ButtonPtr  = std::shared_ptr<Button>;
using ButtonList = std::list<ButtonPtr>;
std::map<std::string, ButtonList> button_list;

//
std::string         current_layer = DefaultLayer;
ButtonPtr           focus_button;
FontDraw::WidgetPtr font;

//
void
text_button(int action, bool enter)
{
  if (enter && action == GLFW_PRESS && !focus_button)
  {
    // カーソルが乗っていない場合でもenterで実行するか
    auto& layer = button_list[current_layer];
    for (auto& btn : layer)
    {
      if (btn->catch_enter)
      {
        // enterを受け付ける場合
        focus_button        = btn;
        focus_button->press = true;
        break;
      }
    }
  }
  if (focus_button)
  {
    if (action == GLFW_RELEASE)
    {
      // ボタンを放したときに実行
      auto btn = focus_button;
      if (btn->press)
        btn->cb();
      btn->press = false;
    }
    else if (action == GLFW_PRESS)
    {
      focus_button->press = true;
    }
  }
}

//
void
draw_box(double lx, double ty, double rx, double by, const Color& col)
{
  auto loc = Graphics::calcLocate(lx, ty, true);
  auto sz  = Graphics::calcLocate(rx, by, true);

  Primitive2D::VertexList vl;
  vl.resize(4);
  vl[0].x = loc.x;
  vl[0].y = loc.y;
  vl[1].x = sz.x;
  vl[1].y = loc.y;
  vl[2].x = sz.x;
  vl[2].y = sz.y;
  vl[3].x = loc.x;
  vl[3].y = sz.y;

  for (auto& v : vl)
  {
    v.r = col.r;
    v.g = col.g;
    v.b = col.b;
  }
  Primitive2D::drawQuads(vl);
}

//
void
print(const std::string msg, double x, double y)
{
  auto loc = Graphics::calcLocate(x, y);
  font->print(msg.c_str(), (float)loc.x, (float)loc.y);
}

} // namespace

//
void
initialize(FontDraw::WidgetPtr f)
{
  font = f;
  Graphics::setTextButtonCallback(text_button);
}

//
void
setButton(std::string caption, double x, double y, PressCallback cb,
          bool catch_enter)
{
  auto& layer = button_list[current_layer];

  int  l  = caption.length() * 24;
  auto lx = x - 20;
  auto rx = x + l + 10;
  auto ty = y - 32 - 10;
  auto by = y + 20;

  auto btn         = std::make_shared<Button>();
  btn->caption     = caption;
  btn->x           = x;
  btn->y           = y;
  btn->lx          = lx;
  btn->rx          = rx;
  btn->ty          = ty;
  btn->by          = by;
  btn->cb          = cb;
  btn->press       = false;
  btn->catch_enter = catch_enter;

  layer.push_back(btn);
}

//
void
setColor(ColorType ct, float r, float g, float b, float a)
{
  auto& col = color_map[ct];
  col.r     = r;
  col.g     = g;
  col.b     = b;
  col.a     = a;
}

// レイヤー切り替え
void
bindLayer(std::string layer)
{
  if (current_layer != layer)
  {
    focus_button.reset();
    current_layer = layer;
  }
}

//
void
clearLayer(std::string layer)
{
  auto& target = button_list[layer];
  target.clear();
  if (current_layer == layer)
  {
    focus_button.reset();
  }
}

//
void
update()
{
  auto& layer = button_list[current_layer];
  auto  mpos  = Graphics::getMousePosition();

  bool focus = false;
  for (auto& btn : layer)
  {
    bool my_focus = focus_button == btn;
    if (!focus)
    {
      if (btn->lx < mpos.x && btn->rx > mpos.x)
        if (btn->ty < mpos.y && btn->by > mpos.y)
        {
          // カーソルが乗っている場合のみ
          my_focus = true;
          focus    = true;
          if (focus_button != btn)
          {
            // フォーカスが切り替わった
            focus_button = btn;
            btn->press   = false;
          }
        }
    }
    // フォーカルによる色選択
    ColorType bg, fg;
    if (my_focus)
    {
      bg = btn->press ? ColorType::PressBG : ColorType::FocusBG;
      fg = btn->press ? ColorType::PressFont : ColorType::FocusFont;
    }
    else
    {
      bg = ColorType::UnFocusBG;
      fg = ColorType::UnFocusFont;
    }
    // 下敷きを描画
    draw_box(btn->lx, btn->ty, btn->rx, btn->by, color_map[bg]);
    // キャプション
    auto fcol = color_map[fg];
    font->setColor(fcol.r, fcol.g, fcol.b, fcol.a);
    print(btn->caption, btn->x, btn->y);
  }
  // どこにもフォーカスしていない(enterによるホールドもされていない)
  if (!focus && focus_button && focus_button->press == false)
    focus_button.reset();
}

} // namespace TextButton
