#include "slidebar.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"

namespace SlideBar
{
using BBox  = BoundingBox::Rect;
using Color = Graphics::Color;

constexpr double pinchW = 20.0;
// constexpr double pinchH = 42.0;

namespace
{
struct BarImpl : public Bar
{
  enum class PStat : int
  {
    UnFocus,
    Left,
    Hold,
    Right,
  };

  double  value;
  double  v_min, v_max;
  double  v_step;
  double  p_x;
  Changed change_func;
  bool    hold;
  PStat   p_stat;
  Type    n_type;

  ~BarImpl() = default;
  bool   getFocus() const override;
  double getNumber() const override { return value; }
  bool   setNumber(double n) override
  {
    bool clip = false;
    if (n < v_min)
    {
      n    = v_min;
      clip = true;
    }
    else if (n > v_max)
    {
      n    = v_max;
      clip = true;
    }
    value = n;
    if (!hold)
      updatePinch(-1.0);
    return clip;
  }
  void setNumberType(Type t) override
  {
    if (n_type != t)
    {
      n_type = t;
      updatePinch(-1.0);
    }
  };
  void setMinMax(double min, double max) override
  {
    v_min = min;
    v_max = max;
  }
  void setStep(double s) override { v_step = s; }
  void setChanged(Changed cf) override { change_func = cf; }

  void draw(bool focus);
  // 値の変更
  void updateValue(double p)
  {
    if (n_type == Type::Integer)
      value = (long)value;
    if (p != value && change_func)
      change_func(value);
  }

  // つまみ
  bool updatePinch(double x)
  {
    double rate = (value - v_min) / (v_max - v_min);
    p_x         = (getWidth() - pinchW) * rate + getX();
    if (hold)
    {
      // 掴んでいるならその位置が値を表す
      auto pw = n_type == Type::Real ? -pinchW * 0.5 : v_step / (v_max - v_min);
      auto rate = (x - getX() + pw) / (getWidth() - pinchW);
      auto prev = value;
      value     = rate * (v_max - v_min) + v_min;
      if (n_type == Type::Integer && value < 0)
        value -= 0.99;
      if (value < v_min)
        value = v_min;
      else if (value > v_max)
        value = v_max;
      updateValue(prev);
    }
    else
    {
      // 掴んでいない場合は位置によって状態が決まる
      if (p_x > x)
        p_stat = PStat::Left;
      else if (p_x + pinchW < x)
        p_stat = PStat::Right;
      else
        p_stat = PStat::Hold;
    }
    return p_stat == PStat::Hold;
  }
  // 値の増減
  void valueStep()
  {
    auto prev = value;
    switch (p_stat)
    {
    case PStat::Hold:
      hold = true;
      break;
    case PStat::Left:
      value -= v_step;
      if (value < v_min)
        value = v_min;
      break;
    case PStat::Right:
      value += v_step;
      if (value > v_max)
        value = v_max;
      break;
    case PStat::UnFocus:
      break;
    }
    updateValue(prev);
  }
  void statClear()
  {
    hold   = false;
    p_stat = PStat::UnFocus;
  }
};

using BarPtr   = std::shared_ptr<BarImpl>;
using ClickAct = Graphics::ClickCallback::Action;
using HoldAct  = Graphics::OffEventCallback::Action;
BarPtr         focus_item;
Layer<BarImpl> layer;

//
void
on_click(ClickAct action, bool enter)
{
  if (auto b = focus_item)
  {
    if (action == ClickAct::Press)
    {
      focus_item->valueStep();
      auto rcb = [](auto act) {
        if (focus_item)
          focus_item->hold = false;
        return act == HoldAct::Release;
      };
      Graphics::disableEvent({rcb});
    }
  }
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
  auto pcol = p_stat == PStat::Hold ? Graphics::Green : Graphics::Cyan;
  Primitive2D::drawBox(p_x, loc.y + 1, p_x + pinchW, btm.y - 1, pcol, true);

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
      bool my_focus = false;
      if (bar->hold)
      {
        bar->updatePinch(mpos.x);
        my_focus = true;
      }
      else
      {
        my_focus = focus_item == bar;
        if (!focus && enabled && bar->checkHit(mpos.x, mpos.y))
        {
          // カーソルが乗っている場合のみ
          my_focus = true;
          focus    = true;
          if (focus_item != bar)
          {
            // フォーカスが切り替わった
            if (focus_item)
              focus_item->statClear();
            bar->statClear();
            focus_item = bar;
          }
          if (Graphics::isEnabledEvent())
            bar->updatePinch(mpos.x);
        }
        else
          bar->statClear();
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
  bar->p_stat = BarImpl::PStat::UnFocus;
  bar->p_x    = 0.0;
  bar->n_type = Type::Real;
  bar->initGeometry(x, y, w, h, -0.01f);
  bar->updatePinch(-1.0);

  auto& item_list = layer.getCurrent();
  item_list.push_back(bar);

  return bar;
}

} // namespace SlideBar
