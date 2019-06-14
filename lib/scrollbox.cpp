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
  bool              focus;
  BoundingBox::Rect bbox;

  void set(double x, double y, double w, double h) override;
  void append(Parts::IDPtr) override;
  void erase(Parts::IDPtr) override;
  void clear() override;
};
using BoxPtr = std::weak_ptr<Box>;
std::list<BoxPtr> box_list;

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
  items.push_back(i);
}
//
void Box::erase(Parts::IDPtr) {}
//
void
Box::clear()
{
  items.clear();
}

} // namespace

//
std::shared_ptr<Base>
create()
{
  auto box = std::make_shared<Box>();
  box_list.push_back(box);
  return box;
}

//
void
update()
{
  auto mpos = Graphics::getMousePosition();

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
      if (bb.check(mpos.x, mpos.y))
        col = Graphics::Green;
      auto loc = bb.getLocate();
      auto btm = bb.getBottom();
      Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, col, false);
    }
  }
}

} // namespace ScrollBox
