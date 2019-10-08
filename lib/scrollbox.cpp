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
using ItemPtr  = std::weak_ptr<Parts::ID>;
using ItemList = std::list<ItemPtr>;
using Color    = Graphics::Color;

//
struct Box : public Base
{
  ItemList items{};
  double   xofs         = 0.0;
  double   yofs         = 0.0;
  double   max_x        = 0.0;
  double   max_y        = 0.0;
  bool     focus        = false;
  bool     xsc_const    = false;
  bool     ysc_const    = false;
  bool     draw_sheet   = false;
  bool     sticky_x     = false;
  bool     sticky_y     = false;
  double   stick_ofs_x  = 0.0;
  double   stick_ofs_y  = 0.0;
  Color    sheet_color  = Graphics::Gray;
  Color    border_color = Graphics::Orange;

  ~Box() = default;
  void set(double x, double y, double w, double h) override;
  void append(Parts::IDPtr) override;
  void erase(Parts::IDPtr) override;
  void clear() override;
  void drawSheet(bool s, Color scol) override;
  void setDepth(float d) override { depth = d; }
  void setFocusBorderColor(Color c) override { border_color = c; }
  void setScrollConstraint(bool sx, bool sy) override
  {
    xsc_const = sx;
    ysc_const = sy;
  }
  void setSticky(bool sx, bool sy) override
  {
    sticky_x    = sx;
    sticky_y    = sy;
    auto ws     = Graphics::getWindowSize();
    stick_ofs_x = sx ? ws.width - (getX() + getWidth()) : 0.0;
    stick_ofs_y = sy ? ws.height - (getY() + getHeight()) : 0.0;
    if (stick_ofs_x < 0.0)
      stick_ofs_x = 0.0;
    if (stick_ofs_y < 0.0)
      stick_ofs_y = 0.0;
  }

  bool   getFocus() const override { return focus; }
  double getPlacementX() const override { return getX() + xofs; }
  double getPlacementY() const override { return getY() + yofs; }

  void scroll_clip();
  void update_sticky();

  void draw(const Color&);
};
using BoxPtr = std::weak_ptr<Box>;
Layer<Box> layer;

BoxPtr focus_box;
bool   initialized = false;

//
void
Box::drawSheet(bool s, Color scol)
{
  draw_sheet  = s;
  sheet_color = scol;
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
Box::update_sticky()
{
  if (sticky_x || sticky_y)
  {
    auto ws = Graphics::getWindowSize();
    auto x  = getX();
    auto y  = getY();
    auto w  = sticky_x ? ws.width - x - stick_ofs_x : getWidth();
    auto h  = sticky_y ? ws.height - y - stick_ofs_y : getHeight();
    bbox    = BoundingBox::Rect{x, y, w, h};
  }
}

//
void
Box::append(Parts::IDPtr i)
{
  i->setParent(this);
  items.push_back(i);

  auto x = i->getX() + i->getWidth() + 20;
  auto y = i->getY() + i->getHeight() + 20;

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
  auto mx = max_x - getWidth();
  auto my = max_y - getHeight();
  if (mx < 0.0)
    mx = 0.0;
  if (my < 0.0)
    my = 0.0;
  if (xofs > 0.0)
    xofs = 0.0;
  else if (xofs < -mx)
    xofs = -mx;
  if (yofs > 0.0)
    yofs = 0.0;
  else if (yofs < -my)
    yofs = -my;
}

//
void
Box::draw(const Color& fcol)
{
  auto loc = bbox.getLocate();
  auto btm = bbox.getBottom();
  Primitive2D::pushDepth(depth);
  if (draw_sheet)
  {
    Primitive2D::setDepth(depth + 0.1f);
    Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, sheet_color, true);
  }
  Primitive2D::setDepth(depth + 0.08f);
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, fcol, false);
  Primitive2D::popDepth();
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
    box->update_sticky();

    auto col = Graphics::White;
    if (!new_focus)
    {
      box->focus = box->checkHit(mpos.x, mpos.y);
      if (box->focus)
        col = box->border_color;
      new_focus = box;
    }
    else
      box->focus = false;
    box->draw(col);
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
