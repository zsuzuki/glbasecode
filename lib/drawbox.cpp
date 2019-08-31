#include "drawbox.h"
#include "bb.h"
#include "gl.h"
#include "primitive2d.h"
#include <cmath>

namespace DrawBox
{
namespace
{
//
struct BoxImpl : public Box
{
  FontDraw::WidgetPtr font;

  double     x, y;
  double     width, height;
  double     ofs_x, ofs_y;
  double     scr_w, scr_h;
  bool       drag_mode;
  const Box* link;

  ~BoxImpl() override = default;
  double getBaseX() const override { return x - ofs_x; }
  double getBaseY() const override { return y - ofs_y; }
  double getWidth() const override { return width; }
  double getHeight() const override { return height; }
  void   begin() override;
  void   end() override;
  void   setDrawSize(double w, double h) override
  {
    scr_w = w - width;
    scr_h = h - height;
    if (scr_w < 0.0)
      scr_w = 0.0;
    if (scr_h < 0.0)
      scr_h = 0.0;
  }
  void setDragScroll(bool e) override { drag_mode = e; }
  void setLink(const Box* other) override { link = other; };
};

using ClickAct      = Graphics::ClickCallback::Action;
using Locate        = Graphics::Locate;
using Vector        = Graphics::Vector;
BoxImpl* valid_drag = nullptr;
BoxImpl* on_drag    = nullptr;
Locate   drag_base;
Vector   drag_scroll;

//
void
BoxImpl::begin()
{
  Graphics::enableScissor(x, y, width, height);
  font->setDrawArea(x, y, width, height);
  auto bbox = BoundingBox::Rect{x, y, width, height};
  auto mpos = Graphics::getMousePosition();
  if (bbox.check(mpos.x, mpos.y))
  {
    valid_drag = this;
    auto sc    = Graphics::getScroll();
    ofs_x += std::abs(sc.x) > 0.1 ? sc.x : 0.0;
    ofs_y += std::abs(sc.y) > 0.1 ? sc.y : 0.0;
  }
  if (drag_mode && (on_drag == this || on_drag == link))
  {
    ofs_x += drag_scroll.x;
    ofs_y += drag_scroll.y;
  }
  ofs_x = ofs_x < 0.0 ? 0.0 : ofs_x > scr_w ? scr_w : ofs_x;
  ofs_y = ofs_y < 0.0 ? 0.0 : ofs_y > scr_h ? scr_h : ofs_y;
}

//
void
BoxImpl::end()
{
  font->clearDrawArea();
  Graphics::disableScissor();
}

//
void
drag_check(ClickAct action, bool)
{
  if (action == ClickAct::Press)
  {
    on_drag   = valid_drag;
    drag_base = Graphics::getMousePosition();
  }
  else if (action == ClickAct::Release)
  {
    on_drag    = nullptr;
    valid_drag = nullptr;
  }
}

} // namespace

//
void
initialize()
{
  Graphics::setClickCallback({drag_check, false});
}

//
void
setup()
{
  valid_drag = nullptr;
  if (on_drag)
  {
    auto p        = Graphics::getMousePosition();
    drag_scroll.x = drag_base.x - p.x;
    drag_scroll.y = drag_base.y - p.y;
    drag_base     = p;
  }
  else
  {
    drag_scroll.x = 0;
    drag_scroll.y = 0;
  }
}

//
BoxPtr
create(FontDraw::WidgetPtr f, int x, int y, int w, int h)
{
  auto p       = std::make_shared<BoxImpl>();
  p->font      = f;
  p->x         = x;
  p->y         = y;
  p->width     = w;
  p->height    = h;
  p->ofs_x     = 0.0;
  p->ofs_y     = 0.0;
  p->scr_w     = w * 2.0;
  p->scr_h     = h * 2.0;
  p->drag_mode = true;
  p->link      = nullptr;

  return p;
}
} // namespace DrawBox