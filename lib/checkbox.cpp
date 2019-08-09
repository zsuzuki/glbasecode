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
  using BBox   = BoundingBox::Rect;
  using Parent = const Parts::ID;
  ~Item()      = default;
  std::string label;
  BBox        bbox;
  double      x, y;
  double      w, h;
  double      ox, oy;
  double      length;
  bool        value;
  Parent*     parent;

  double getX() const override { return x + ox; }
  double getY() const override { return y + oy; }
  int    getWidth() const override { return w; }
  int    getHeight() const override { return h; }
  void   setParent(const Parts::ID* p) override { parent = p; }

  void setText(std::string) override;
  void setValue(bool v) override { value = v; }
  bool getValue() const override { return value; }

  operator bool() const override { return getValue(); }

  void draw(bool);
  bool update();
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
    }
  }
}

//
bool
Item::update()
{
  float depth = 0.0f;
  if (parent)
  {
    ox    = parent->getPlacementX();
    oy    = parent->getPlacementY();
    depth = parent->getDepth() - 0.1f;
  }
  Primitive2D::setDepth(depth);
  font->setDepth(depth - 0.05f);
  bbox = BBox{x + ox, y + oy, w, h};
  return parent ? parent->inRect(bbox) : true;
}

//
void
Item::draw(bool focus)
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

  auto loc = bbox.getLocate();
  auto btm = bbox.getBottom();
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, Graphics::Gray, false);
  auto pos = Graphics::calcLocate(loc.x + 20, loc.y + 42);
  font->setColor(value ? Graphics::White : Graphics::Gray);
  font->print(label.c_str(), pos.x, pos.y);

  Graphics::disableScissor();
  font->clearDrawArea();
}

//
void
Item::setText(std::string str)
{
  double l  = CodeConv::U8Length2(str.c_str()) * 21.0;
  auto   rx = x + l + 20 + 20;
  auto   by = y + 20 + 32 + 10;

  label  = str;
  length = l;
  w      = rx - x;
  h      = by - y;
  bbox   = BoundingBox::Rect{x, y, w, h};
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
  item->ox     = 0.0;
  item->oy     = 0.0;
  item->parent = nullptr;
  item->value  = sw;
  item->setText(str);

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
  bool  focus     = false;
  for (auto& item : item_list)
  {
    if (item->update() == false)
      continue;
    bool my_focus = focus_item == item;
    if (!focus)
    {
      if (item->bbox.check(mpos.x, mpos.y))
      {
        // on cursor
        my_focus = true;
        focus    = true;
        if (focus_item != item)
        {
          focus_item = item;
        }
      }
    }
    item->draw(my_focus);
  }
  if (!focus && focus_item)
    focus_item.reset();
  font->popDepth();
  Primitive2D::popDepth();
}

} // namespace CheckBox