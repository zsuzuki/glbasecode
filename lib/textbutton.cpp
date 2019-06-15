#include "textbutton.h"
#include "bb.h"
#include "codeconv.h"
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
//
std::string         current_layer = DefaultLayer;
FontDraw::WidgetPtr font;

//
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
struct Button : public Parts::ID
{
  using BBox   = BoundingBox::Rect;
  using Parent = const Parts::ID;
  ~Button()    = default;
  std::string   caption;
  BBox          bbox;
  double        x, y;
  double        w, h;
  double        ox, oy;
  double        length;
  PressCallback cb;
  bool          catch_enter;
  bool          press;
  Parent*       parent;

  double getX() const override { return x + ox; }
  double getY() const override { return y + oy; }
  int    getWidth() const override { return w; }
  int    getHeight() const override { return h; }
  void   setParent(const Parts::ID* p) override { parent = p; }
  bool   update()
  {
    float depth    = 0.0f;
    bool  en_focus = true;
    if (parent)
    {
      ox       = parent->getPlacementX();
      oy       = parent->getPlacementY();
      en_focus = parent->getFocus();
      depth    = parent->getDepth() - 0.1f;
    }
    Primitive2D::setDepth(depth);
    font->setDepth(depth - 0.05f);
    bbox = BBox{x + ox, y + oy, w, h};
    return en_focus;
  }
};
using ButtonPtr  = std::shared_ptr<Button>;
using ButtonList = std::list<ButtonPtr>;
std::map<std::string, ButtonList> button_list;
ButtonPtr                         focus_button;

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
draw_box(const Button& btn, const Color& col)
{
  auto bl  = btn.bbox.getLocate();
  auto loc = Graphics::calcLocate(bl.x, bl.y, true);
  auto sz  = Graphics::calcLocate(bl.x + btn.w, bl.y + btn.h, true);

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
  Graphics::setClickCallback({text_button, true});
}

//
ID
setButton(std::string caption, double x, double y, PressCallback cb,
          bool catch_enter)
{
  auto& layer = button_list[current_layer];

  double l  = CodeConv::U8Length2(caption.c_str()) * 21.0;
  auto   rx = x + l + 20 + 20;
  auto   by = y + 20 + 32 + 10;

  auto btn         = std::make_shared<Button>();
  btn->caption     = caption;
  btn->length      = l;
  btn->x           = x;
  btn->y           = y;
  btn->w           = rx - x;
  btn->h           = by - y;
  btn->ox          = 0.0;
  btn->oy          = 0.0;
  btn->bbox        = BoundingBox::Rect{x, y, btn->w, btn->h};
  btn->cb          = cb;
  btn->press       = false;
  btn->catch_enter = catch_enter;

  layer.push_back(btn);

  return btn;
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
erase(ID id)
{
  auto& target = button_list[current_layer];
  for (auto p = target.begin(); p != target.end(); p++)
  {
    if (*p == id)
    {
      target.erase(p);
      break;
    }
  }
}

//
void
update()
{
  auto& layer = button_list[current_layer];
  auto  mpos  = Graphics::getMousePosition();

  Primitive2D::pushDepth(0.01f);
  font->pushDepth(0.0f);
  bool focus = false;
  for (auto& btn : layer)
  {
    bool ef       = btn->update();
    bool my_focus = focus_button == btn;
    if (!focus)
    {
      if (ef && btn->bbox.check(mpos.x, mpos.y))
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
    draw_box(*btn, color_map[bg]);
    // キャプション
    auto fcol = color_map[fg];
    font->setColor({fcol.r, fcol.g, fcol.b, fcol.a});
    print(btn->caption, btn->getX() + 20, btn->getY() + 42);
  }
  // どこにもフォーカスしていない(enterによるホールドもされていない)
  if (!focus && focus_button && focus_button->press == false)
    focus_button.reset();
  Primitive2D::popDepth();
  font->popDepth();
}

} // namespace TextButton
