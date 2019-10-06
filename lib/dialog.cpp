#include "dialog.h"
#include "bb.h"
#include "codeconv.h"
#include "gl.h"
#include "primitive2d.h"
#include "texture2d.h"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <list>
#include <map>

namespace Dialog
{
namespace
{
using Tex2D = Texture2D::ImagePtr;

FontDraw::WidgetPtr font;
std::vector<Tex2D>  icon_list;

// 選択状態
enum class Select : int
{
  None,
  OK,
  Cancel
};

//
struct Body : public Base
{
  using Message = std::list<std::string>;
  using OfsList = std::vector<double>;

  BBox       bb_ok;
  BBox       bb_cancel;
  double     width, height;
  DecideFunc df_ok;
  DecideFunc df_cancel;
  Message    message;
  double     max_length;
  OfsList    offset;
  bool       need_cancel;
  Select     sel_state;
  int        icon;
  int        icon_height;

  //
  ~Body() = default;
  double getX() const override { return x; }
  double getY() const override { return y; }
  int    getWidth() const override { return width; }
  int    getHeight() const override { return height + icon_height; }
  float  getDepth() const override { return depth; }
  void   setOK(DecideFunc f) override { df_ok = f; }
  void   setCancel(DecideFunc f) override { df_cancel = f; }
  void   setIcon(int id) override
  {
    auto& ip    = icon_list[id];
    icon        = id;
    icon_height = ip->getHeight();
  }

  void set_message(std::string msg)
  {
    boost::split(message, msg, boost::is_any_of("\n"));
    offset.reserve(message.size());
    offset.resize(0);
    max_length = 0;
    for (auto& s : message)
    {
      auto l = CodeConv::U8Length2(s.c_str());
      if (l > max_length)
        max_length = l;
      offset.push_back(l);
    }
  }
  void update();
  void draw();
};

using BodyPtr  = std::shared_ptr<Body>;
using ClickAct = Graphics::OffEventCallback::Action;

BodyPtr current_dialog;

//
bool
on_click(ClickAct action)
{
  if (!current_dialog)
    return false;

  bool act_ok = false;
  bool act_cl = false;
  if (action == ClickAct::Click)
  {
    // クリックの場合、明示的にボタンを押す
    if (current_dialog->sel_state == Select::OK)
      act_ok = true;
    else if (current_dialog->sel_state == Select::Cancel)
      act_cl = true;
  }
  else if (current_dialog->need_cancel)
  {
    // キャンセルボタンがある場合、それぞれの役割を判定
    act_ok = action == ClickAct::EnterKey;
    act_cl = action == ClickAct::EscapeKey;
  }
  else
  {
    // OKボタンのみの場合、なんか押されたらそれで決定
    if (action == ClickAct::EnterKey || action == ClickAct::EscapeKey)
      act_ok = true;
  }

  if (act_ok)
  {
    // OKが押された
    if (current_dialog->df_ok)
      current_dialog->df_ok(true);
    current_dialog.reset();
  }
  else if (act_cl)
  {
    // Cancelが押された
    if (current_dialog->df_cancel)
      current_dialog->df_cancel(false);
    current_dialog.reset();
  }

  return act_ok || act_cl;
}

//
void
Body::update()
{
  auto w  = getWidth();
  auto h  = getHeight();
  auto sz = Graphics::getWindowSize();
  x       = (sz.width - w) * 0.5;
  y       = (sz.height - h) * 0.5;

  auto mpos = Graphics::getMousePosition();

  if (need_cancel)
  {
    // キャンセルボタン有
    auto dx   = x + w * 0.3 - (30 + 21 * 3);
    auto dy   = y + h - 130;
    bb_cancel = BBox{dx, dy, 21 * 6 + 60, 60};
    dx        = x + w * 0.7 - 51;
    bb_ok     = BBox{dx, dy, 102, 60};
    if (bb_cancel.check(mpos.x, mpos.y))
      sel_state = Select::Cancel;
    else
      sel_state = bb_ok.check(mpos.x, mpos.y) ? Select::OK : Select::None;
  }
  else
  {
    // キャンセルボタン無
    auto dx   = x + w * 0.5 - 51;
    auto dy   = y + h - 130;
    bb_ok     = BBox{dx, dy, 102, 60};
    sel_state = bb_ok.check(mpos.x, mpos.y) ? Select::OK : Select::None;
  }
}

//
void
Body::draw()
{
  auto w = getWidth();
  auto h = getHeight();
  Primitive2D::drawBox(x, y, x + w, y + h, Graphics::DarkGray, true);
  Primitive2D::setDepth(-0.92f);
  Primitive2D::drawBox(x, y, x + w, y + h, Graphics::White, false);
  font->setColor(Graphics::White);
  auto dy = y + 75;
  if (icon >= 0)
  {
    auto ws     = Graphics::getWindowSize();
    auto dset   = Texture2D::DrawSet();
    auto loc    = Graphics::calcLocate(x + w * 0.5, dy);
    dset.image  = icon_list[icon];
    dset.height = icon_height * 2.0 / ws.width;
    dset.width  = dset.image->getWidth() * 2.0 / ws.width;
    dset.align  = Texture2D::Align::CenterTop;
    dset.depth  = -0.94f;
    dset.x      = loc.x;
    dset.y      = loc.y;
    Texture2D::draw(dset);
    dy += icon_height;
  }
  dy += 75;
  auto ofs = offset.begin();
  for (auto& m : message)
  {
    auto o = (max_length - *ofs) * 0.5 * 21.0;
    auto p = Graphics::calcLocate(x + 300 + o, dy);
    font->print(m.c_str(), p.x, p.y);
    dy += 50;
    ofs++;
  }

  auto loc  = bb_ok.getLocate();
  auto btm  = bb_ok.getBottom();
  auto bcol = sel_state == Select::OK ? Graphics::Green : Graphics::White;
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, bcol, false);
  auto bfpos = Graphics::calcLocate(loc.x + 30, loc.y + 42);
  font->print("OK", bfpos.x, bfpos.y);
  if (need_cancel)
  {
    loc  = bb_cancel.getLocate();
    btm  = bb_cancel.getBottom();
    bcol = sel_state == Select::Cancel ? Graphics::Red : Graphics::White;
    Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, bcol, false);
    auto bfpos = Graphics::calcLocate(loc.x + 30, loc.y + 42);
    font->print("CANCEL", bfpos.x, bfpos.y);
  }
}

} // namespace

//
void
initialize(FontDraw::WidgetPtr f)
{
  font = f;
}

//
int
registIcon(const char* fname)
{
  int  idx = icon_list.size();
  auto p   = Texture2D::create(fname);
  icon_list.push_back(p);
  return idx;
}

//
ID
create(std::string msg, bool need_cancel)
{
  auto dlg = std::make_shared<Body>();
  dlg->set_message(msg);
  dlg->width       = dlg->max_length * font->getSizeX() + 300 * 2;
  dlg->height      = dlg->message.size() * font->getSizeY() + 330;
  dlg->need_cancel = need_cancel;
  dlg->sel_state   = Select::None;
  dlg->icon        = -1;
  dlg->icon_height = 0;
  return dlg;
}

//
bool
open(ID d)
{
  if (current_dialog)
    return false;

  current_dialog = std::dynamic_pointer_cast<Body>(d);
  if (!current_dialog)
    return false;

  Graphics::disableEvent({on_click});
  return true;
}

//
void
close()
{
  if (!current_dialog)
    return;

  if (current_dialog->df_cancel)
    current_dialog->df_cancel(false);
  current_dialog.reset();
  Graphics::enableEvent();
}

//
void
update()
{
  if (!current_dialog)
    return;

  Primitive2D::pushDepth(-0.9f);
  font->pushDepth(-0.91f);
  current_dialog->update();
  current_dialog->draw();
  Primitive2D::popDepth();
  font->popDepth();
}

} // namespace Dialog