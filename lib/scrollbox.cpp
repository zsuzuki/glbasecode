#include "scrollbox.h"
#include "bb.h"
#include "gl.h"
#include "primitive2d.h"
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

  ItemList          items;
  double            xofs;
  double            yofs;
  double            depth;
  bool              focus;
  bool              draw_sheet;
  BoundingBox::Rect bbox;
  Graphics::Color   sheet_color;

  void set(double x, double y, double w, double h) override;
  void append(Parts::IDPtr) override;
  void erase(Parts::IDPtr) override;
  void clear() override;
  void drawSheet(bool s, Graphics::Color scol) override;
  void setDepth(float d) override;

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
};
using BoxPtr = std::weak_ptr<Box>;
std::list<BoxPtr> box_list;

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
  bbox = BoundingBox::Rect{x, y, w, h};
}
//
void
Box::append(Parts::IDPtr i)
{
  i->setParent(this);
  items.push_back(i);
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
}

//
void
key_callback(int key, int scancode, int action, int mods)
{
  if (focus_box.expired())
    return;
  auto box = focus_box.lock();
  if (box->focus == false)
    return;
  if (key == GLFW_KEY_UP)
    box->yofs += 10;
  else if (key == GLFW_KEY_DOWN)
    box->yofs -= 10;
}
void
scroll_callback(double xofs, double yofs)
{
  if (focus_box.expired())
    return;
  auto box = focus_box.lock();
  if (box->focus == false)
    return;
  box->xofs += xofs;
  box->yofs += yofs;
}

} // namespace

//
std::shared_ptr<Base>
create()
{
  auto box = std::make_shared<Box>();
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
  auto mpos = Graphics::getMousePosition();

  std::shared_ptr<Box> new_focus;
  for (auto bi = box_list.begin(); bi != box_list.end(); bi++)
  {
    auto& bp = *bi;
    if (bp.expired())
    {
      bi = box_list.erase(bi);
    }
    else
    {
      auto  box = bp.lock();
      auto& bb  = box->bbox;

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
        Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, box->sheet_color,
                             true);
      }
      Primitive2D::setDepth(box->depth - 0.01f);
      Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, col, false);
      Primitive2D::popDepth();
    }
  }
  focus_box = new_focus;
}

} // namespace ScrollBox
