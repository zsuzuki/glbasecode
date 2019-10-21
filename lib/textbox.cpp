#include "textbox.h"
#include "bb.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"
#include "slidebar.h"
#include "text.h"
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>

namespace TextBox
{
namespace
{
using Color = Graphics::Color;

constexpr double BaseX = 12.0;
constexpr double BaseY = 24.0;

//
struct ItemImpl : public Item
{
  std::string  text;
  std::string  place_holder;
  size_t       max_length  = 99;
  Color        font_color  = Graphics::White;
  Color        bg_color    = Graphics::Black;
  Color        ph_color    = Graphics::Gray;
  double       ofs_y       = 0.0;
  double       sl_num      = 0.0;
  bool         draw_border = true;
  bool         on_edit     = false;
  int          sl_prec     = 0;
  InputStyle   input_style = InputStyle::Text;
  SlideBar::ID slider;
  Pulldown::ID pulldown;

  ItemImpl()  = default;
  ~ItemImpl() = default;

  void setPulldown(Pulldown::ID pd) override
  {
    pulldown = pd;
    if (pulldown)
      pulldown->setParent(this);
  }

  void        setPlaceHolder(std::string ph) override { place_holder = ph; }
  void        drawBorder(bool draw) override { draw_border = draw; }
  std::string getText() const override { return text; }
  void        setFontColor(Graphics::Color fc) override { font_color = fc; }
  void        setBGColor(Graphics::Color bc) override { bg_color = bc; }
  void setPlaceHolderColor(Graphics::Color pc) override { ph_color = pc; }
  void setMaxLength(size_t ml) override { max_length = ml; }
  void setInputStyle(InputStyle st) override { input_style = st; }
  void setSlider(Parts::IDPtr s, int pr) override
  {
    slider      = std::dynamic_pointer_cast<SlideBar::Bar>(s);
    sl_prec     = pr;
    input_style = InputStyle::Number;
  }

  void draw();
  void drawCursor();
  void updateLink();
};
using ItemImplPtr = std::shared_ptr<ItemImpl>;
using ClickAct    = Graphics::ClickCallback::Action;

TextInput::Buffer   text_buffer;
ItemImplPtr         focus_input;
ItemImplPtr         edit_input;
FontDraw::WidgetPtr font;
Layer<ItemImpl>     layer;

//
void
input_finish(ItemImplPtr& item)
{
  if (TextInput::onInput())
    TextInput::finish();
  if (item)
  {
    item->on_edit = false;
    item.reset();
  }
}

//
void
on_click(ClickAct action, bool enter)
{
  if (action == ClickAct::Press)
  {
    if (edit_input)
    {
      // 編集中の場合
      auto& pd = edit_input->pulldown;
      if (pd)
      {
        // プルダウンがオープンしているときはそちらの処理しかしない
        if (pd->isOpened())
          return;
      }
    }

    if (focus_input)
    {
      // フォーカスしているアイテムに対して
      if (focus_input != edit_input)
      {
        // フォーカスの違う所をクリックしたら、編集し直し
        input_finish(edit_input);
        TextInput::setBuffer(text_buffer, focus_input->text);
        TextInput::start(text_buffer, focus_input->max_length);
        TextInput::setInputStyle(focus_input->input_style);
        TextInput::setPulldown(focus_input->pulldown);
        focus_input->on_edit = true;
        edit_input           = focus_input;
      }
      // クリックした位置にカーソルを設定する
      auto mpos = Graphics::getMousePosition();
      auto xp   = (mpos.x - focus_input->getX() - 10.0) / 21.0;
      if (xp >= 0)
        TextInput::setIndex(xp);
    }
  }
}

//
void
ItemImpl::updateLink()
{
  if (slider)
  {
    auto n = slider->getNumber();
    if (n != sl_num)
    {
      std::ostringstream out;
      out << std::fixed << std::setprecision(sl_prec) << n;
      text   = out.str();
      sl_num = n;
      if (on_edit && TextInput::onInput())
      {
        // TextInput::setBuffer(text_buffer, text);
        input_finish(edit_input);
        edit_input.reset();
      }
    }
    else if (text.empty() == false)
    {
      try
      {
        auto nn = std::stod(text);
        if (nn != sl_num)
        {
          if (slider->setNumber(nn) == false)
            sl_num = nn;
        }
      }
      catch (std::invalid_argument& e)
      {
        // '-'だけとかは何もしない
      }
      catch (std::out_of_range& e)
      {
        // 不正な値もやっぱり何もしない
      }
    }
  }
}

//
void
ItemImpl::drawCursor()
{
  auto loc = bbox.getLocate();
  loc.x += BaseX;
  loc.y += ofs_y;
  auto ofs = TextInput::getIndexPos();
  auto l1  = Graphics::calcLocate(loc.x + ofs.left, loc.y, true);
  auto l2  = Graphics::calcLocate(loc.x + ofs.right, loc.y, true);
  static Primitive2D::VertexList ul = {
      {0.0f, 0.0f, 0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f, 0.0f},
  };
  ul[0].x = l1.x;
  ul[0].y = l1.y;
  ul[1].x = l2.x;
  ul[1].y = l2.y;
  Primitive2D::drawLine(ul, 4.0f);
}

//
void
ItemImpl::draw()
{
  if (parent)
  {
    auto px = parent->getX();
    auto py = parent->getY();
    auto pw = parent->getWidth();
    auto ph = parent->getHeight();
    Graphics::enableScissor(px, py, pw, ph);
    font->setDrawArea(px, py, pw, ph);
  }
  auto loc = bbox.getLocate();
  auto btm = bbox.getBottom();
  if (draw_border)
    Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, Graphics::White, false);
  if (on_edit)
  {
    drawCursor();
    if (TextInput::onInput())
      text = TextInput::get();
  }

  auto pos = Graphics::calcLocate(loc.x + BaseX, loc.y + ofs_y);
  if (text.empty())
  {
    font->setColor(ph_color);
    font->print(place_holder.c_str(), pos.x, pos.y);
  }
  else
  {
    font->setColor(font_color);
    font->print(text.c_str(), pos.x, pos.y);
  }

  Graphics::disableScissor();
  font->clearDrawArea();
}

} // namespace

//
void
bindLayer(std::string l)
{
  if (layer.bind(l))
  {
    input_finish(edit_input);
    focus_input.reset();
    edit_input.reset();
  }
}
//
void
clearLayer(std::string l)
{
  layer.clear(l);
}

//
void
eraseItem(ItemPtr item)
{
  auto ip = std::dynamic_pointer_cast<ItemImpl>(item);
  if (ip)
    layer.erase(ip);
}

//
ItemPtr
create(std::string t, double x, double y, double w, double h)
{
  if (w == 0.0)
  {
    w = t.length() * 21;
    if (w == 0.0)
      w = 16 * 21;
    w += 40;
    x -= 20.0;
  }
  if (h == 0.0)
  {
    y -= 48;
    h = 48;
  }
  auto item = std::make_shared<ItemImpl>();
  auto o    = (h - BaseY) * 0.5;
  if (o >= 0.0)
    item->ofs_y = BaseY + o;
  item->text = t;
  item->initGeometry(x, y, w, h);

  auto& item_list = layer.getCurrent();
  item_list.push_back(item);
  return item;
}

//
void
initialize(FontDraw::WidgetPtr f)
{
  font = f;
  Graphics::setClickCallback({on_click, false});
}

//
void
update()
{
  Primitive2D::pushDepth(0.01f);
  font->pushDepth(0.0f);
  auto  mpos      = Graphics::getMousePosition();
  auto& item_list = layer.getCurrent();
  focus_input     = ItemImplPtr{};
  for (auto& item : item_list)
  {
    bool no_input = true;
    item->update([&](bool enabled) {
      no_input = false;
      item->updateLink();
      if (enabled && item->checkHit(mpos.x, mpos.y))
      {
        if (focus_input != item)
          focus_input = item;
      }
      item->draw();
    });
    if (no_input && edit_input == item)
      input_finish(edit_input);
  }
  Primitive2D::popDepth();
  font->popDepth();
}

} // namespace TextBox
