#include "dialog.h"
#include "bb.h"
#include "codeconv.h"
#include "gl.h"
#include "primitive2d.h"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <list>
#include <map>

namespace Dialog
{
namespace
{
FontDraw::WidgetPtr font;

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
  using BBox    = BoundingBox::Rect;
  using Message = std::list<std::string>;
  using OfsList = std::vector<double>;

  BBox       bb_ok;
  BBox       bb_cancel;
  double     x, y;
  double     width, height;
  DecideFunc df_ok;
  DecideFunc df_cancel;
  Message    message;
  double     max_length;
  float      depth;
  OfsList    offset;
  bool       need_cancel;
  Select     sel_state;

  //
  double getX() const override { return x; }
  double getY() const override { return y; }
  int    getWidth() const override { return width; }
  int    getHeight() const override { return height; }
  float  getDepth() const override { return depth; }
  void   setOK(DecideFunc f) override { df_ok = f; }
  void   setCancel(DecideFunc f) override { df_cancel = f; }

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
using ClickAct = Graphics::ClickCallback::Action;

BodyPtr current_dialog;

//
void
on_click(ClickAct action, bool)
{
  if (action == ClickAct::Press && current_dialog)
  {
    switch (current_dialog->sel_state)
    {
    case Select::OK:
      if (current_dialog->df_ok)
        current_dialog->df_ok(true);
      current_dialog.reset();
      break;
    case Select::Cancel:
      if (current_dialog->df_cancel)
        current_dialog->df_cancel(false);
      current_dialog.reset();
      break;
    default:
      break;
    }
  }
}

//
void
Body::update()
{
  auto sz = Graphics::getWindowSize();
  x       = (sz.width - width) * 0.5;
  y       = (sz.height - height) * 0.5;

  auto mpos = Graphics::getMousePosition();

  if (need_cancel)
  {
    // キャンセルボタン有
    auto dx   = x + width * 0.3 - (30 + 21 * 3);
    auto dy   = y + height - 130;
    bb_cancel = BBox{dx, dy, 21 * 6 + 60, 60};
    dx        = x + width * 0.7 - 51;
    bb_ok     = BBox{dx, dy, 102, 60};
    if (bb_ok.check(mpos.x, mpos.y))
      sel_state = Select::OK;
    else
      sel_state =
          bb_cancel.check(mpos.x, mpos.y) ? Select::Cancel : Select::None;
  }
  else
  {
    // キャンセルボタン無
    auto dx   = x + width * 0.5 - 51;
    auto dy   = y + height - 130;
    bb_ok     = BBox{dx, dy, 102, 60};
    sel_state = bb_ok.check(mpos.x, mpos.y) ? Select::OK : Select::None;
  }
}

//
void
Body::draw()
{
  Primitive2D::drawBox(x, y, x + width, y + height, Graphics::DarkGray, true);
  Primitive2D::setDepth(-0.11f);
  Primitive2D::drawBox(x, y, x + width, y + height, Graphics::White, false);
  font->setColor(Graphics::White);
  auto dy  = y + 150;
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
  Graphics::setClickCallback({on_click, true});
}

//
ID
create(std::string msg, bool need_cancel)
{
  auto dlg = std::make_shared<Body>();
  dlg->set_message(msg);
  dlg->width       = dlg->max_length * 21 + 300 * 2;
  dlg->height      = dlg->message.size() * 40 + 400;
  dlg->need_cancel = need_cancel;
  dlg->sel_state   = Select::None;
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
}

//
void
update()
{
  if (!current_dialog)
    return;

  Primitive2D::pushDepth(-0.1f);
  font->pushDepth(-0.11f);
  current_dialog->update();
  current_dialog->draw();
  Primitive2D::popDepth();
  font->popDepth();
}

} // namespace Dialog