#include "textbutton.h"
#include "bb.h"
#include "codeconv.h"
#include "font.h"
#include "gl.h"
#include "layer.h"
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
FontDraw::WidgetPtr font;

using Color = Graphics::Color;
const Color DarkGray{0.3f, 0.3f, 0.3f, 1.0f};

std::map<ColorType, Color> color_map = {
    {ColorType::UnFocusBG, DarkGray},
    {ColorType::FocusBG, Graphics::Gray},
    {ColorType::PressBG, Graphics::White},
    {ColorType::UnFocusFont, Graphics::White},
    {ColorType::FocusFont, Graphics::White},
    {ColorType::PressFont, DarkGray},
};

//
struct Button : public Base
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
  Color         c_ufbg;
  Color         c_fbg;
  Color         c_pbg;
  Color         c_uff;
  Color         c_ff;
  Color         c_pf;
  Pulldown::ID  pulldown;

  void   setCaption(std::string c) override { caption = c; }
  double getX() const override { return x + ox; }
  double getY() const override { return y + oy; }
  int    getWidth() const override { return w; }
  int    getHeight() const override { return h; }
  void   setParent(const Parts::ID* p) override { parent = p; }
  bool   getFocus() const override;
  void   setColor(ColorType ct, Graphics::Color col) override
  {
    switch (ct)
    {
    case ColorType::UnFocusBG:
      c_ufbg = col;
      break;
    case ColorType::FocusBG:
      c_fbg = col;
      break;
    case ColorType::PressBG:
      c_pbg = col;
      break;
    case ColorType::UnFocusFont:
      c_uff = col;
      break;
    case ColorType::FocusFont:
      c_ff = col;
      break;
    case ColorType::PressFont:
      c_pf = col;
      break;
    }
  }
  Color getColor(ColorType ct) const
  {
    auto col = Color();
    switch (ct)
    {
    case ColorType::UnFocusBG:
      col = c_ufbg;
      break;
    case ColorType::FocusBG:
      col = c_fbg;
      break;
    case ColorType::PressBG:
      col = c_pbg;
      break;
    case ColorType::UnFocusFont:
      col = c_uff;
      break;
    case ColorType::FocusFont:
      col = c_ff;
      break;
    case ColorType::PressFont:
      col = c_pf;
      break;
    }
    return col;
  }
  void setPulldown(Pulldown::ID pd) override
  {
    pulldown = pd;
    if (pulldown)
      pulldown->setParent(this);
  }

  std::pair<bool, bool> update()
  {
    float depth    = 0.0f;
    float inrect   = true;
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
    if (parent)
      inrect = parent->inRect(bbox);
    return std::make_pair(inrect, en_focus);
  }

  void draw(ColorType fg, ColorType bg);
};
using ButtonPtr = std::shared_ptr<Button>;
using ClickAct  = Graphics::ClickCallback::Action;
ButtonPtr     focus_button;
Layer<Button> layer;

//
void
text_button(ClickAct action, bool enter)
{
  if (enter && action == ClickAct::Press && !focus_button)
  {
    // カーソルが乗っていない場合でもenterで実行するか
    auto& button_list = layer.getCurrent();
    for (auto& btn : button_list)
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
    if (action == ClickAct::Release)
    {
      // ボタンを放したときに実行
      auto btn = focus_button;
      if (btn->press)
      {
        auto& pd = btn->pulldown;
        if (pd)
        {
          if (pd->isOpened())
            pd->close();
          else
            pd->open();
        }
        btn->cb();
      }
      btn->press = false;
    }
    else if (action == ClickAct::Press)
    {
      focus_button->press = true;
    }
  }
}

//
void
print(const std::string& msg, double x, double y)
{
  auto loc = Graphics::calcLocate(x, y);
  font->print(msg.c_str(), (float)loc.x, (float)loc.y);
}

//
bool
Button::getFocus() const
{
  return focus_button.get() == this;
}

//
void
Button::draw(ColorType fg, ColorType bg)
{
  if (parent)
  {
    auto px = parent->getX();
    auto py = parent->getY();
    auto pw = parent->getWidth();
    auto ph = parent->getHeight();
    Graphics::enableScissor(px, py, pw, ph);
    font->setDrawArea(px, py, pw, ph);
  }

  // 下敷きを描画
  auto bcol = getColor(bg);
  auto loc  = bbox.getLocate();
  auto btm  = bbox.getBottom();
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, bcol, true);

  // キャプション
  auto fcol = getColor(fg);
  font->setColor(fcol);
  print(caption, getX() + 20, getY() + 42);

  Graphics::disableScissor();
  font->clearDrawArea();
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
  btn->parent      = nullptr;
  btn->catch_enter = catch_enter;
  btn->c_ufbg      = color_map[ColorType::UnFocusBG];
  btn->c_fbg       = color_map[ColorType::FocusBG];
  btn->c_pbg       = color_map[ColorType::PressBG];
  btn->c_uff       = color_map[ColorType::UnFocusFont];
  btn->c_ff        = color_map[ColorType::FocusFont];
  btn->c_pf        = color_map[ColorType::PressFont];

  auto& button_list = layer.getCurrent();
  button_list.push_back(btn);

  return btn;
}

//
void
setDefaultColor(ColorType ct, Graphics::Color c)
{
  color_map[ct] = c;
}

// レイヤー切り替え
void
bindLayer(std::string l)
{
  if (layer.bind(l))
    focus_button.reset();
}

//
void
clearLayer(std::string l)
{
  layer.clear(l);
  focus_button.reset();
}

//
void
erase(ID id)
{
  auto ip = std::dynamic_pointer_cast<Button>(id);
  if (ip)
    layer.erase(ip);
}

//
void
update()
{
  auto& button_list = layer.getCurrent();
  auto  mpos        = Graphics::getMousePosition();

  Primitive2D::pushDepth(0.01f);
  font->pushDepth(0.0f);
  bool focus = false;
  for (auto& btn : button_list)
  {
    auto ef = btn->update();
    if (!ef.first)
      continue;
    bool my_focus = focus_button == btn;
    bool inbox    = btn->bbox.check(mpos.x, mpos.y);
    if (!focus)
    {
      if (ef.second && inbox)
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
    btn->draw(fg, bg);
  }
  // どこにもフォーカスしていない(enterによるホールドもされていない)
  if (!focus && focus_button && focus_button->press == false)
    focus_button.reset();
  Primitive2D::popDepth();
  font->popDepth();
}

} // namespace TextButton
