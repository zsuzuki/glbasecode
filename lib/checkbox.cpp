#include "checkbox.h"
#include "bb.h"
#include "codeconv.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"
#include <iostream>
#include <list>
#include <map>

namespace CheckBox
{

namespace
{
//
FontDraw::WidgetPtr font;

//
struct Item : public Base
{
  using Color = Graphics::Color;
  struct Info
  {
    std::string label;
    double      length;
    Color       color;
  };
  ~Item() = default;
  Info    on_info;
  Info    off_info;
  double  length;
  bool    value;
  Changed change_func;

  void setChanged(Changed ch) override { change_func = ch; }
  void setText(std::string) override;
  void setOffText(std::string) override;
  void setValue(bool v) override { value = v; }
  bool getValue() const override { return value; }

  void setOnColor(Graphics::Color c) override { on_info.color = c; }
  void setOffColor(Graphics::Color c) override { off_info.color = c; }

  double setBBox(const std::string& s);
  void   draw(bool);
};

using ItemPtr  = std::shared_ptr<Item>;
using ClickAct = Graphics::ClickCallback::Action;
ItemPtr     focus_item;
Layer<Item> layer;

//
void
on_click(ClickAct action, bool enter)
{
  if (action == ClickAct::Press)
  {
    if (focus_item)
    {
      focus_item->value = !focus_item->value;
      if (focus_item->change_func)
        focus_item->change_func(focus_item->value);
    }
  }
}

//
void
Item::draw(bool focus)
{
  float ldepth = depth;
  if (parent)
  {
    auto px = parent->getX();
    auto py = parent->getY();
    auto pw = parent->getWidth();
    auto ph = parent->getHeight();
    ldepth += parent->getDepth();
    Graphics::enableScissor(px, py, pw, ph);
    font->setDrawArea(px, py, pw, ph);
  }

  Primitive2D::setDepth(ldepth);
  font->setDepth(ldepth - 0.02f);

  auto loc = bbox.getLocate();
  auto btm = bbox.getBottom();
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, Graphics::Gray, false);

  auto& info   = value ? on_info : off_info;
  auto  offset = (length - info.length) * 0.5;
  auto  pos    = Graphics::calcLocate(loc.x + 20 + offset, loc.y + 42);
  font->setColor(info.color);
  font->print(info.label.c_str(), pos.x, pos.y);

  Graphics::disableScissor();
  font->clearDrawArea();
}

//
double
Item::setBBox(const std::string& s)
{
  double l = CodeConv::U8Length2(s.c_str()) * 21.0;
  if (l > length)
  {
    auto rx = x + l + 20 + 20;
    auto by = y + 20 + 32 + 10;

    length = l;
    w      = rx - x;
    h      = by - y;
    bbox   = BoundingBox::Rect{x, y, w, h};
  }
  return l;
}

//
void
Item::setText(std::string str)
{
  on_info.label  = str;
  on_info.length = setBBox(str);
}

//
void
Item::setOffText(std::string str)
{
  off_info.label  = str;
  off_info.length = setBBox(str);
}

} // namespace

//
void
initialize(FontDraw::WidgetPtr f)
{
  font = f;
  Graphics::setClickCallback({on_click, false});
}
//
ID
create(std::string str, double x, double y, bool sw)
{
  auto item = std::make_shared<Item>();

  item->x      = x;
  item->y      = y;
  item->value  = sw;
  item->length = 0;

  item->on_info.color  = Graphics::White;
  item->off_info.color = Graphics::Gray;
  item->setText(str);
  item->setOffText(str);

  layer.append(item);
  return item;
}
//
void
bindLayer(std::string l)
{
  layer.bind(l);
}
//
void
clearLayer(std::string l)
{
  layer.clear(l);
}
//
void
erase(ID i)
{
  auto ip = std::dynamic_pointer_cast<Item>(i);
  if (ip)
    layer.erase(ip);
}

//
void
update()
{
  Primitive2D::pushDepth(0);
  font->pushDepth(0);
  font->setColor(Graphics::White);
  auto  mpos      = Graphics::getMousePosition();
  auto& item_list = layer.getCurrent();
  bool  focus     = !Graphics::isEnabledEvent();
  for (auto& item : item_list)
  {
    item->update([&](bool enabled) {
      bool my_focus = focus_item == item;
      if (!focus && enabled && item->checkHit(mpos.x, mpos.y))
      {
        // on cursor
        my_focus = true;
        focus    = true;
        if (focus_item != item)
        {
          focus_item = item;
        }
      }
      item->draw(my_focus);
    });
  }
  if (!focus && focus_item)
    focus_item.reset();
  font->popDepth();
  Primitive2D::popDepth();
}

} // namespace CheckBox