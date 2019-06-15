#include "scrollbox.h"
#include "bb.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"
#include <iostream>
#include <list>

namespace ScrollBox
{
namespace
{
//
struct Box : public Base
{
  using ItemPtr  = std::weak_ptr<Parts::ID>;
  using ItemList = std::list<ItemPtr>;

  ItemList          items{};
  BoundingBox::Rect bbox{};
  double            xofs        = 0.0;
  double            yofs        = 0.0;
  double            depth       = 0.0;
  double            max_x       = 0.0;
  double            max_y       = 0.0;
  bool              focus       = false;
  bool              xsc_const   = false;
  bool              ysc_const   = false;
  bool              draw_sheet  = false;
  Graphics::Color   sheet_color = Graphics::Gray;

  void set(double x, double y, double w, double h) override;
  void append(Parts::IDPtr) override;
  void erase(Parts::IDPtr) override;
  void clear() override;
  void drawSheet(bool s, Graphics::Color scol) override;
  void setDepth(float d) override;
  void setScrollConstraint(bool sx, bool sy) override
  {
    xsc_const = sx;
    ysc_const = sy;
  }

  double getX() const override { return bbox.getLeftX(); }
  double getY() const override { return bbox.getTopY(); }
  int    getWidth() const override { return bbox.getWidth(); }
  int    getHeight() const override { return bbox.getHeight(); }
  float  getDepth() const override { return depth; }
  bool   getFocus() const override { return focus; }
  double getPlacementX() const override { return getX() + xofs; }
  double getPlacementY() const override { return getY() + yofs; }
  bool   inRect(const BoundingBox::Rect& r) const override
  {
    return bbox.checkHit(r);
  }

  void scroll_clip();
};
using BoxPtr = std::weak_ptr<Box>;
Layer<Box> layer;

BoxPtr focus_box;
bool   initialized = false;

//
void
Box::drawSheet(bool s, Graphics::Color scol)
{
  draw_sheet  = s;
  sheet_color = scol;
}
//
void
Box::setDepth(float d)
{
  depth = d;
}

//
void
Box::set(double x, double y, double w, double h)
{
  bbox  = BoundingBox::Rect{x, y, w, h};
  max_x = 0.0;
  max_y = 0.0;
}
//
void
Box::append(Parts::IDPtr i)
{
  i->setParent(this);
  items.push_back(i);

  auto width  = getWidth() - i->getWidth() - 20;
  auto height = getHeight() - i->getHeight() - 20;
  auto x      = i->getX() - width;
  auto y      = i->getY() - height;

  if (x > 0.0)
  {
    if (max_x < x)
      max_x = x;
  }
  if (y > 0.0)
  {
    if (max_y < y)
      max_y = y;
  }
}
//
void
Box::erase(Parts::IDPtr i)
{
  for (auto it = items.begin(); it != items.end(); it++)
  {
    auto ptr = (*it).lock();
    if (ptr == i)
    {
      items.erase(it);
      break;
    }
  }
}
//
void
Box::clear()
{
  items.clear();
  xofs  = 0.0;
  yofs  = 0.0;
  max_x = 0.0;
  max_y = 0.0;
}
//
void
Box::scroll_clip()
{
  if (xofs > 0.0)
    xofs = 0.0;
  else if (xofs < -max_x)
    xofs = -max_x;
  if (yofs > 0.0)
    yofs = 0.0;
  else if (yofs < -max_y)
    yofs = -max_y;
}

//
void
key_callback(int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS && action != GLFW_REPEAT)
    return;
  if (focus_box.expired())
    return;
  auto box = focus_box.lock();
  if (box->focus == false)
    return;
  if (box->ysc_const == false)
  {
    if (key == GLFW_KEY_UP)
      box->yofs += 40;
    else if (key == GLFW_KEY_DOWN)
      box->yofs -= 40;
  }
  if (box->xsc_const == false)
  {
    if (key == GLFW_KEY_RIGHT)
      box->xofs += 40;
    else if (key == GLFW_KEY_LEFT)
      box->xofs -= 40;
  }
  box->scroll_clip();
}
void
scroll_callback(double xofs, double yofs)
{
  if (focus_box.expired())
    return;
  auto box = focus_box.lock();
  if (box->focus == false)
    return;
  if (box->xsc_const == false)
    box->xofs += xofs * 6.0;
  if (box->ysc_const == false)
    box->yofs += yofs * 6.0;
  box->scroll_clip();
}

} // namespace

//
std::shared_ptr<Base>
create()
{
  auto  box      = std::make_shared<Box>();
  auto& box_list = layer.getCurrent();
  box_list.push_back(box);
  if (initialized == false)
  {
    Graphics::setScrollBoxFunction(scroll_callback, key_callback);
    initialized = true;
  }
  return box;
}

//
void
update()
{
  auto  mpos     = Graphics::getMousePosition();
  auto& box_list = layer.getCurrent();

  std::shared_ptr<Box> new_focus;
  for (auto box : box_list)
  {
    auto& bb = box->bbox;

    Graphics::Color col = Graphics::White;
    if (!new_focus)
    {
      box->focus = bb.check(mpos.x, mpos.y);
      if (box->focus)
        col = Graphics::Green;
      new_focus = box;
    }
    else
      box->focus = false;
    auto loc = bb.getLocate();
    auto btm = bb.getBottom();
    Primitive2D::pushDepth(box->depth);
    if (box->draw_sheet)
    {
      Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, box->sheet_color, true);
    }
    Primitive2D::setDepth(box->depth - 0.01f);
    Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, col, false);
    Primitive2D::popDepth();
  }
  focus_box = new_focus;
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
erase(SBoxPtr box)
{
  auto bp = std::dynamic_pointer_cast<Box>(box);
  if (bp)
    layer.erase(bp);
}

} // namespace ScrollBox
