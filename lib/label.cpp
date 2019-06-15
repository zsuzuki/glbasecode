#include "label.h"
#include "bb.h"
#include "codeconv.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"
#include <iostream>
#include <list>
#include <map>

namespace Label
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
  Parent*     parent;
  Color       fgcol;
  Color       bgcol;

  double getX() const override { return x + ox; }
  double getY() const override { return y + oy; }
  int    getWidth() const override { return w; }
  int    getHeight() const override { return h; }
  void   setParent(const Parts::ID* p) override { parent = p; }

  void setText(std::string l) override;
  void setFontColor(Graphics::Color col) override { fgcol = col; }
  void setBGColor(Graphics::Color col) override { bgcol = col; }

  void draw();
  bool update();
};

using ItemPtr = std::shared_ptr<Item>;
Layer<Item> layer;

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
Item::draw()
{
  auto loc = bbox.getLocate();
  auto btm = bbox.getBottom();
  if (bgcol.a > 0.0)
    Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, bgcol, true);
  font->setColor(fgcol);
  auto pos = Graphics::calcLocate(loc.x + 20, loc.y + 42);
  font->print(label.c_str(), pos.x, pos.y);
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
}
//
ID
create(std::string str, double x, double y, Color fg, Color bg)
{
  auto item = std::make_shared<Item>();

  item->x      = x;
  item->y      = y;
  item->ox     = 0.0;
  item->oy     = 0.0;
  item->parent = nullptr;
  item->fgcol  = fg;
  item->bgcol  = bg;
  item->setText(str);

  auto& item_list = layer.getCurrent();
  item_list.push_back(item);

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
  auto& item_list = layer.getCurrent();
  for (auto& item : item_list)
  {
    if (item->update() == false)
      continue;
    item->draw();
  }
  font->popDepth();
  Primitive2D::popDepth();
}

} // namespace Label