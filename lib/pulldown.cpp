#include "pulldown.h"
#include "bb.h"
#include "codeconv.h"
#include "gl.h"
#include "primitive2d.h"
#include <list>

namespace Pulldown
{
namespace
{
FontDraw::WidgetPtr font;
//
struct Item : public Base
{
  using BBox   = BoundingBox::Rect;
  using Parent = const Parts::ID;

  double      x, y;
  double      width, height;
  Parent*     parent;
  std::string filter;
  int         select_index;
  int         mouse_focus;
  bool        opened;
  BBox        bbox;
  List        items;
  size_t      nb_disp;
  size_t      disp_top;
  Selected    selected_func;

  ~Item() override = default;
  double getX() const override { return x; }
  double getY() const override { return y; }
  int    getWidth() const override { return width; }
  int    getHeight() const override { return height; }
  void   setParent(const Parts::ID* p) override { parent = p; }
  void   setFilter(std::string f) override { filter = f; }
  size_t getIndex() const override { return select_index; }
  void   open() override;
  void   close() override { opened = false; }
  bool   isOpened() const override { return opened; }
  void   setSelected(Selected sf) override { selected_func = sf; }

  operator bool() const { return isOpened(); }

  bool updateAndDraw(const Graphics::Locate&);
};
using ItemPtr  = std::shared_ptr<Item>;
using ClickAct = Graphics::ClickCallback::Action;
std::list<ItemPtr> item_list;

ItemPtr focus_item;

//
void
on_click(ClickAct action, bool enter)
{
  if (action == ClickAct::Press && focus_item)
  {
    int mf = focus_item->mouse_focus;
    if (mf != -1)
    {
      if (focus_item->selected_func)
        focus_item->selected_func(mf + focus_item->disp_top);
      focus_item->close();
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
void
Item::open()
{
  opened = true;
  if (parent)
  {
    x = parent->getX() + 20;
    y = parent->getY() + parent->getHeight();
  }
  bbox = BoundingBox::Rect{x, y, width, height};
}

//
bool
Item::updateAndDraw(const Graphics::Locate& mpos)
{
  auto depth = parent->getDepth() - 0.1f;

  // 下敷きを描画
  auto bcol = Graphics::DarkGray;
  auto loc  = bbox.getLocate();
  auto btm  = bbox.getBottom();
  Primitive2D::setDepth(depth - 0.1f);
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, bcol, true);
  auto lcol = Graphics::White;
  Primitive2D::setDepth(depth - 0.11f);
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, lcol, false);

  // フォーカス
  int mf = -1;
  if (bbox.check(mpos.x, mpos.y))
  {
    int oy = (int)(mpos.y - loc.y - 8.0) / 42;
    if (oy >= 0 && oy < nb_disp)
    {
      mf = oy;
      if (mouse_focus != mf)
        select_index = mf + disp_top;
    }
  }
  mouse_focus = mf;

  // 選択
  {
    double y    = loc.y + (select_index - disp_top) * 42.0 + 8.0;
    auto   fcol = Graphics::LightGray;
    Primitive2D::setDepth(depth - 0.12f);
    Primitive2D::drawBox(loc.x + 5, y, btm.x - 5, y + 42, fcol, true);
  }

  // 文字列描画
  font->setDepth(depth - 0.13f);
  for (int i = 0; i < nb_disp; i++)
  {
    auto& str  = items[i + disp_top];
    auto  fcol = Graphics::White;
    font->setColor(fcol);
    print(str, getX() + 20, getY() + i * 42.0 + 42.0);
  }

  return mf != -1;
}

} // namespace

//
void
initialize(FontDraw::WidgetPtr f)
{
  font = f;
  Graphics::setClickCallback({on_click, true});
}

//
void
clear()
{
  item_list.clear();
}

//
void
update()
{
  Primitive2D::pushDepth(0.01f);
  font->pushDepth(0.0f);
  auto mpos  = Graphics::getMousePosition();
  focus_item = ItemPtr{};
  for (auto& i : item_list)
  {
    if (*i && i->updateAndDraw(mpos))
    {
      focus_item = i;
    }
  }
  Primitive2D::popDepth();
  font->popDepth();
}

//
ID
create(List&& l, size_t nb_disp)
{
  auto item          = std::make_shared<Item>();
  item->x            = 0.0;
  item->y            = 0.0;
  item->parent       = nullptr;
  item->filter       = "";
  item->select_index = 0;
  item->opened       = false;
  item->mouse_focus  = -1;

  int ml = 0;
  for (auto& s : l)
  {
    auto len = CodeConv::U8Length2(s.c_str()) * 21.0;
    if (len > ml)
      ml = len;
  }
  size_t nd = l.size();
  if (nd > nb_disp)
    nd = nb_disp;
  item->width    = ml + 20 + 20;
  item->height   = nd * 42.0 + 20;
  item->nb_disp  = nd;
  item->disp_top = 0;
  item->items    = l;

  item_list.push_back(item);

  return item;
}

//
void
erase(ID i)
{
  for (auto p = item_list.begin(); p != item_list.end(); p++)
  {
    if (*p == i)
    {
      item_list.erase(p);
      break;
    }
  }
}

} // namespace Pulldown