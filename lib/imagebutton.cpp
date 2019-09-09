#include "imagebutton.h"
#include "bb.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"
#include "texture2d.h"
#include <list>
#include <map>

namespace ImageButton
{

namespace
{
using ImagePtr = Texture2D::ImagePtr;
using BBox     = BoundingBox::Rect;
using Parent   = const Parts::ID;
using Color    = Graphics::Color;

//
struct ButtonImpl : public Button
{
  ImagePtr      focus_image;
  ImagePtr      unfocus_image;
  PressCallback callback;
  BBox          bbox;
  double        x, y;
  double        ox, oy;
  double        w, h;
  double        depth;
  bool          press;
  Parent*       parent;
  Color         focus_color;
  Color         unfocus_color;

  ~ButtonImpl() = default;
  double getX() const override { return x + ox; }
  double getY() const override { return y + oy; }
  int    getWidth() const override { return w; }
  int    getHeight() const override { return h; }
  float  getDepth() const override { return depth; }
  void   setParent(const Parts::ID* p) override { parent = p; }
  bool   getFocus() const override;
  void   setFocusIcon(const char* fname) override
  {
    focus_image = new_image(fname);
  }
  void setUnFocusIcon(const char* fname) override
  {
    unfocus_image = new_image(fname);
  }
  void setFocusColor(Graphics::Color c) override { focus_color = c; };
  void setUnFocusColor(Graphics::Color c) override { unfocus_color = c; };
  void setImageWidth(double) override {}
  void setImageHeight(double) override {}

  void draw(bool focus);

  Texture2D::ImagePtr new_image(const char* fname)
  {
    auto img = Texture2D::create(fname);
    return img;
  }

  std::pair<bool, bool> update()
  {
    float inrect   = true;
    bool  en_focus = true;
    if (parent)
    {
      ox       = parent->getPlacementX();
      oy       = parent->getPlacementY();
      en_focus = parent->getFocus();
    }
    bbox = BBox{x + ox, y + oy, w, h};
    if (parent)
      inrect = parent->inRect(bbox);
    return std::make_pair(inrect, en_focus);
  }
};
//
using ButtonPtr = std::shared_ptr<ButtonImpl>;
using ClickAct  = Graphics::ClickCallback::Action;
ButtonPtr         focus_button;
Layer<ButtonImpl> layer;

//
void
button_click(ClickAct action, bool enter)
{
  if (focus_button)
  {
    auto btn = focus_button;
    if (action == ClickAct::Release)
    {
      // ボタンを放したときに実行
      if (btn->press)
        btn->callback();
      btn->press = false;
    }
    else if (action == ClickAct::Press)
    {
      btn->press = true;
    }
  }
}

//
void
ButtonImpl::draw(bool focus)
{
  float ldepth = depth;
  if (parent)
  {
    auto px = parent->getX();
    auto py = parent->getY();
    auto pw = parent->getWidth();
    auto ph = parent->getHeight();
    ldepth += parent->getDepth();
    Texture2D::setDrawArea(px, py, pw, ph);
  }

  Texture2D::DrawSet dset;
  dset.image  = focus ? focus_image : unfocus_image;
  auto loc    = bbox.getLocate();
  auto btm    = bbox.getBottom();
  auto lt     = Graphics::calcLocate(loc.x, loc.y);
  auto rb     = Graphics::calcLocate(btm.x, btm.y);
  dset.x      = lt.x;
  dset.y      = lt.y;
  dset.width  = rb.x - lt.x;
  dset.height = lt.y - rb.y;
  dset.depth  = ldepth;
  dset.align  = Texture2D::Align::LeftTop;
  dset.color  = focus ? focus_color : unfocus_color;
  dset.aspect = false;
  Texture2D::draw(dset);

  Texture2D::clearDrawArea();
}

//
bool
ButtonImpl::getFocus() const
{
  return focus_button.get() == this;
}

}; // namespace

//
void
initialize()
{
  Graphics::setClickCallback({button_click, true});
}

//
void
bindLayer(std::string l)
{
  if (layer.bind(l))
    focus_button.reset();
}

//
void
clearLayer(std::string l)
{
  layer.clear(l);
  focus_button.reset();
}

//
void
erase(ID id)
{
  auto ip = std::dynamic_pointer_cast<ButtonImpl>(id);
  if (ip)
    layer.erase(ip);
}

//
void
update()
{
  auto& button_list = layer.getCurrent();
  auto  mpos        = Graphics::getMousePosition();
  bool  focus       = !Graphics::isEnabledEvent();
  for (auto& btn : button_list)
  {
    auto ef = btn->update();
    if (!ef.first)
      continue;
    bool my_focus = focus_button == btn;
    if (!focus)
    {
      bool inbox = btn->bbox.check(mpos.x, mpos.y);
      if (ef.second && inbox)
      {
        // カーソルが乗っている場合のみ
        my_focus = true;
        focus    = true;
        if (focus_button != btn)
        {
          // フォーカスが切り替わった
          focus_button = btn;
          btn->press   = false;
        }
      }
    }
    btn->draw(my_focus);
  }
  // どこにもフォーカスしていない(enterによるホールドもされていない)
  if (!focus && focus_button && focus_button->press == false)
    focus_button.reset();
}

//
ID
create(const char* in, double x, double y, PressCallback cb, bool c_ent)
{
  auto img = Texture2D::create(in);
  if (!img)
    return ID{};
  auto btn = std::make_shared<ButtonImpl>();

  btn->focus_image   = img;
  btn->unfocus_image = img;
  btn->x             = x;
  btn->y             = y;
  btn->w             = img->getWidth();
  btn->h             = img->getHeight();
  btn->ox            = 0.0;
  btn->oy            = 0.0;
  btn->depth         = -0.05f;
  btn->bbox          = BoundingBox::Rect{x, y, btn->w, btn->h};
  btn->callback      = cb;
  btn->parent        = nullptr;
  btn->press         = false;
  btn->focus_color   = Graphics::White;
  btn->unfocus_color = Graphics::White;

  auto& button_list = layer.getCurrent();
  button_list.push_back(btn);

  return btn;
}

} // namespace ImageButton
