#include "slidebar.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"

namespace SlideBar
{
using BBox  = BoundingBox::Rect;
using Color = Graphics::Color;

constexpr double pinchW = 20.0;
constexpr double pinchH = 42.0;

namespace
{
struct BarImpl : public Bar
{
  double  value;
  double  v_min, v_max;
  double  v_step;
  Changed change_func;
  bool    hold;

  ~BarImpl() = default;
  bool   getFocus() const override;
  double getNumber() const override { return value; }
  void   setNumber(double n) override { value = n; }
  void   setMinMax(double min, double max) override
  {
    v_min = min;
    v_max = max;
  }
  void setStep(double s) override { v_step = s; }
  void setChanged(Changed cf) override { change_func = cf; }

  void draw(bool focus);
};

using BarPtr   = std::shared_ptr<BarImpl>;
using ClickAct = Graphics::ClickCallback::Action;
BarPtr         focus_item;
Layer<BarImpl> layer;

//
void
on_click(ClickAct action, bool enter)
{
}

//
bool
BarImpl::getFocus() const
{
  return focus_item.get() == this;
}

//
void
BarImpl::draw(bool focus)
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
  }

  Primitive2D::setDepth(ldepth);
  auto bgcol = focus ? Graphics::Orange : Graphics::Gray;
  auto loc   = bbox.getLocate();
  auto btm   = bbox.getBottom();
  if (bgcol.a > 0.0)
    Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, bgcol, false);

  Primitive2D::setDepth(ldepth - 0.02f);
  double rate = (value - v_min) / (v_max - v_min);
  double xp   = (getWidth() - pinchW) * rate + loc.x;
  auto   pcol = Graphics::Cyan;
  Primitive2D::drawBox(xp, loc.y + 1, xp + pinchW, btm.y - 1, pcol, true);

  Graphics::disableScissor();
}

} // namespace

//
void
initialize()
{
  Graphics::setClickCallback({on_click, false});
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
  auto ip = std::dynamic_pointer_cast<BarImpl>(i);
  if (ip)
    layer.erase(ip);
}

//
void
update()
{
  Primitive2D::pushDepth(0);
  auto  mpos      = Graphics::getMousePosition();
  auto& item_list = layer.getCurrent();
  bool  focus     = !Graphics::isEnabledEvent();
  for (auto& bar : item_list)
  {
    bar->update([&](bool enabled) {
      bool my_focus = focus_item == bar;
      if (!focus && enabled && bar->checkHit(mpos.x, mpos.y))
      {
        // カーソルが乗っている場合のみ
        my_focus = true;
        focus    = true;
        if (focus_item != bar)
        {
          // フォーカスが切り替わった
          focus_item = bar;
          bar->hold  = false;
        }
      }
      bar->draw(my_focus);
    });
  }
  if (!focus && focus_item && focus_item->hold == false)
    focus_item.reset();

  Primitive2D::popDepth();
}

//
ID
create(double x, double y, double w, double h)
{
  auto bar    = std::make_shared<BarImpl>();
  bar->hold   = false;
  bar->value  = 0.0;
  bar->v_min  = 0.0;
  bar->v_max  = 1.0;
  bar->v_step = 0.1;
  bar->initGeometry(x, y, w, h, -0.01f);

  auto& item_list = layer.getCurrent();
  item_list.push_back(bar);

  return bar;
}

} // namespace SlideBar
