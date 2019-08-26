#include "drawbox.h"
#include "gl.h"
#include "primitive2d.h"

namespace DrawBox
{
namespace
{
//
struct BoxImpl : public Box
{
  FontDraw::WidgetPtr font;

  double x, y;
  double width, height;
  double ofs_x, ofs_y;
  double scr_w, scr_h;

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
};

//
void
BoxImpl::begin()
{
  Graphics::enableScissor(x, y, width, height);
  font->setDrawArea(x, y, width, height);
  auto sc = Graphics::getScroll();
  ofs_x += sc.x;
  ofs_y += sc.y;
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

} // namespace

//
BoxPtr
create(FontDraw::WidgetPtr f, int x, int y, int w, int h)
{
  auto p    = std::make_shared<BoxImpl>();
  p->font   = f;
  p->x      = x;
  p->y      = y;
  p->width  = w;
  p->height = h;
  p->ofs_x  = 0.0;
  p->ofs_y  = 0.0;
  p->scr_w  = w * 2.0;
  p->scr_h  = h * 2.0;

  return p;
}
} // namespace DrawBox