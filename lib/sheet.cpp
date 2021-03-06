#include "sheet.h"
#include "bb.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"

namespace Sheet
{
namespace
{
using Color = Graphics::Color;

//
struct SheetImpl : public Base
{
  Color border;
  Color fill;

  ~SheetImpl() = default;
  void setBorderColor(Graphics::Color c) override { border = c; }
  void setFillColor(Graphics::Color c) override { fill = c; }
  void setSize(double wd, double ht) override
  {
    width  = wd;
    height = ht;
  }
  void setDepth(float d) override { depth = d; }

  void draw()
  {
    auto d = depth;
    if (parent)
    {
      auto px = parent->getX();
      auto py = parent->getY();
      auto pw = parent->getWidth();
      auto ph = parent->getHeight();
      d += parent->getDepth();
      Graphics::enableScissor(px, py, pw, ph);
    }

    auto loc = bbox.getLocate();
    auto btm = bbox.getBottom();
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
    sh->update([&](bool) { sh->draw(); });
  }
  Primitive2D::popDepth();
}

//
ID
create(double x, double y, double w, double h)
{
  auto sh    = std::make_shared<SheetImpl>();
  sh->border = Graphics::White;
  sh->fill   = Graphics::DarkGray;
  sh->initGeometry(x, y, w, h);

  auto& sheet_list = layer.getCurrent();
  sheet_list.push_back(sh);

  return sh;
}

} // namespace Sheet
