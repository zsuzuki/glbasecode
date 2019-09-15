#include "sheet.h"
#include "bb.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"

namespace Sheet
{
namespace
{
using Parent = const Parts::ID;
using Color  = Graphics::Color;
using BBox   = BoundingBox::Rect;

//
struct SheetImpl : public Base
{
  double  x, y;
  double  width, height;
  double  ox, oy;
  double  depth;
  Parent* parent;
  Color   border;
  Color   fill;

  ~SheetImpl() = default;
  double getX() const override { return x + ox; }
  double getY() const override { return y + oy; }
  int    getWidth() const override { return width; }
  int    getHeight() const override { return height; }
  float  getDepth() const override { return depth; }
  void   setParent(const Parts::ID* p) override { parent = p; }
  void   setBorderColor(Graphics::Color c) override { border = c; }
  void   setFillColor(Graphics::Color c) override { fill = c; }
  void   setDepth(float d) override { depth = d; }
  void   setSize(double w, double h) override
  {
    width  = w;
    height = h;
  }

  void draw()
  {
    auto d = depth;
    if (parent)
    {
      ox      = parent->getPlacementX();
      oy      = parent->getPlacementY();
      auto px = parent->getX();
      auto py = parent->getY();
      auto pw = parent->getWidth();
      auto ph = parent->getHeight();
      d += parent->getDepth();
      Graphics::enableScissor(px, py, pw, ph);
    }
    auto bbox = BBox{x + ox, y + oy, width, height};
    auto loc  = bbox.getLocate();
    auto btm  = bbox.getBottom();
    Primitive2D::setDepth(d);
    if (border.a > 0.0f)
      Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, border, false);
    if (fill.a > 0.0f)
      Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, fill, true);

    Graphics::disableScissor();
  }
};
Layer<SheetImpl> layer;

} // namespace

//
void
initialize()
{
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
erase(ID id)
{
  auto ip = std::dynamic_pointer_cast<SheetImpl>(id);
  if (ip)
    layer.erase(ip);
}

//
void
update()
{
  Primitive2D::pushDepth(0.0f);
  auto& sheet_list = layer.getCurrent();
  for (auto& sh : sheet_list)
  {
    sh->draw();
  }
  Primitive2D::popDepth();
}

//
ID
create(double x, double y, double w, double h)
{
  auto sh    = std::make_shared<SheetImpl>();
  sh->x      = x;
  sh->y      = y;
  sh->width  = w;
  sh->height = h;
  sh->ox     = 0.0;
  sh->oy     = 0.0;
  sh->border = Graphics::White;
  sh->fill   = Graphics::DarkGray;
  sh->parent = nullptr;
  sh->depth  = 0.05f;

  auto& sheet_list = layer.getCurrent();
  sheet_list.push_back(sh);

  return sh;
}

} // namespace Sheet
