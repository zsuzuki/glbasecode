#include "notification.h"
#include "codeconv.h"
#include "font.h"
#include "gl.h"
#include "primitive2d.h"
#include <chrono>
#include <list>

namespace Notification
{
using namespace std::chrono;
namespace
{
using Color = Graphics::Color;
using Time  = steady_clock::time_point;
using MSEC  = time_point<milliseconds>;

FontDraw::WidgetPtr font;

//
struct MsgImpl : public Msg
{
  Color       border_col;
  Color       fill_col;
  Color       font_col;
  std::string message;
  Time        start;
  int         disp_time;
  double      disp_y;

  ~MsgImpl() = default;
  void setBorderColor(Graphics::Color c) override { border_col = c; }
  void setFillColor(Graphics::Color c) override { fill_col = c; }
  void setFontColor(Graphics::Color c) override { font_col = c; }

  double draw(double tgt_y)
  {
    disp_y += (tgt_y - disp_y) * 0.05;
    auto ws = Graphics::getWindowSize();
    auto l  = CodeConv::U8Length2(message.c_str()) * font->getSizeX();
    auto x  = ws.width - (l + 130.0);
    auto rx = ws.width - 10.0;
    auto y  = disp_y + 5.0;
    auto by = disp_y + 75.0;
    if (fill_col.a > 0.0f)
    {
      Primitive2D::setDepth(-0.96f);
      Primitive2D::drawBox(x, y, rx, by, fill_col, true);
    }
    if (border_col.a > 0.0f)
    {
      Primitive2D::setDepth(-0.98f);
      Primitive2D::drawBox(x, y, rx, by, border_col, false);
    }
    font->setColor(font_col);
    auto loc = Graphics::calcLocate(x + 60, y + 48);
    font->print(message.c_str(), loc.x, loc.y);
    return tgt_y + 80.0;
  }
};
using MsgPtr = std::shared_ptr<MsgImpl>;
std::list<MsgPtr> msg_list;

} // namespace

//
ID
notify(std::string msg, int dispms)
{
  auto i        = std::make_shared<MsgImpl>();
  i->message    = msg;
  i->start      = steady_clock::now();
  i->disp_time  = dispms;
  i->disp_y     = msg_list.size() * 80;
  i->border_col = Graphics::White;
  i->fill_col   = Graphics::DarkGray;
  i->font_col   = Graphics::White;

  msg_list.push_back(i);
  return i;
}

//
void
initialize(FontDraw::WidgetPtr f)
{
  font = f;
}

//
void
update()
{
  Primitive2D::pushDepth(0.0f);
  font->pushDepth(-0.98f);
  double target_y = 0.0;
  for (auto mi = msg_list.begin(); mi != msg_list.end();)
  {
    auto m = *mi;
    auto p = mi++;
    auto n = steady_clock::now();
    auto d = duration_cast<milliseconds>(n - m->start);
    if (d > milliseconds(m->disp_time))
    {
      // end
      msg_list.erase(p);
    }
    target_y = m->draw(target_y);
  }
  Primitive2D::popDepth();
  font->popDepth();
}

} // namespace Notification
