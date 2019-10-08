#include "label.h"
#include "bb.h"
#include "codeconv.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"
#include "slidebar.h"
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
  ~Item() = default;
  std::string  label;
  double       length;
  Color        fgcol;
  Color        bgcol;
  Parts::IDPtr slider;

  void setText(std::string l) override;
  void setFontColor(Graphics::Color col) override { fgcol = col; }
  void setBGColor(Graphics::Color col) override { bgcol = col; }
  void setSlider(Parts::IDPtr s) override { slider = s; }

  void draw();
  void updateLink()
  {
    auto s = std::dynamic_pointer_cast<SlideBar::Bar>(slider);
    if (s)
    {
      setText(std::to_string(s->getNumber()));
    }
  }
};

using ItemPtr = std::shared_ptr<Item>;
Layer<Item> layer;

//
void
Item::draw()
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
  if (bgcol.a > 0.0)
    Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, bgcol, true);
  font->setColor(fgcol);
  auto pos = Graphics::calcLocate(loc.x + 20, loc.y + 42);
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
  initGeometry(x, y, rx - x, by - y);
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
  auto item   = std::make_shared<Item>();
  item->fgcol = fg;
  item->bgcol = bg;
  item->initGeometry(x, y);
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
    item->update([&](bool) {
      item->updateLink();
      item->draw();
    });
  }
  font->popDepth();
  Primitive2D::popDepth();
}

} // namespace Label