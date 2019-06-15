#include "textbox.h"
#include "bb.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"
#include "text.h"
#include <iostream>
#include <list>
#include <map>

namespace TextBox
{
namespace
{

constexpr double BaseX = 12.0;
constexpr double BaseY = 24.0;

//
class ItemImpl : public Item
{
public:
  std::string       text;
  std::string       place_holder;
  BoundingBox::Rect bbox;
  size_t            max_length;
  Graphics::Color   font_color;
  Graphics::Color   bg_color;
  Graphics::Color   ph_color;
  double            ofs_y;
  bool              draw_border;
  bool              on_edit;

  ItemImpl(std::string t, double x, double y, double w, double h)
      : Item(), text(t), bbox(x, y, w, h), max_length(99),
        font_color(Graphics::White), ph_color(Graphics::Gray),
        bg_color(Graphics::Black), ofs_y(0.0), draw_border(true),
        on_edit(false){};
  ~ItemImpl() = default;

  void        setPlaceHolder(std::string ph) override { place_holder = ph; }
  void        drawBorder(bool draw) override { draw_border = draw; }
  std::string getText() const override { return text; }
  void        setFontColor(Graphics::Color fc) override { font_color = fc; }
  void        setBGColor(Graphics::Color bc) override { bg_color = bc; }
  void setPlaceHolderColor(Graphics::Color pc) override { ph_color = pc; }
  void setMaxLength(size_t ml) override { max_length = ml; }

  void drawBG();
  void drawCursor();
};
using ItemImplPtr = std::shared_ptr<ItemImpl>;

TextInput::Buffer   text_buffer;
ItemImplPtr         focus_input;
ItemImplPtr         edit_input;
FontDraw::WidgetPtr font;
Layer<ItemImpl>     layer;

//
void
ItemImpl::drawBG()
{
  auto lt = bbox.getLocate();
  auto rb = bbox.getBottom();
  auto l1 = Graphics::calcLocate(lt.x, lt.y, true);
  auto l2 = Graphics::calcLocate(rb.x, rb.y, true);

  static Primitive2D::VertexList ul;
  ul.resize(5);
  for (auto& v : ul)
  {
    v.r = 1.0f;
    v.g = 1.0f;
    v.b = 1.0f;
    v.a = 1.0f;
  }
  ul[0].x = ul[3].x = ul[4].x = l1.x;
  ul[0].y = ul[1].y = ul[4].y = l1.y;
  ul[1].x = ul[2].x = l2.x;
  ul[2].y = ul[3].y = l2.y;
  Primitive2D::drawLine(ul, 2.0f);
}

//
void
on_click(int action, bool enter)
{
  if (action == GLFW_PRESS)
  {
    if (focus_input != edit_input)
    {
      if (edit_input)
        edit_input->on_edit = false;
      if (TextInput::onInput())
        TextInput::finish();
      TextInput::setBuffer(text_buffer, focus_input->text);
      TextInput::start(text_buffer, 99);
      focus_input->on_edit = true;
      edit_input           = focus_input;
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
set_focus(ItemImplPtr item)
{
  if (focus_input != item)
  {
    focus_input = item;
  }
}

} // namespace

//
void
bindLayer(std::string l)
{
  if (layer.bind(l))
  {
    focus_input.reset();
    if (edit_input)
    {
      edit_input->on_edit = false;
      edit_input.reset();
      if (TextInput::onInput())
        TextInput::finish();
    }
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
    w = t.length() * BaseY + 30;
    x -= 20.0;
  }
  if (h == 0.0)
  {
    y -= 40;
    h = 40;
  }
  auto item = std::make_shared<ItemImpl>(t, x, y, w, h);
  auto o    = (h - BaseY) * 0.5;
  if (o >= 0.0)
    item->ofs_y = BaseY + o;

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
  auto& item_list = layer.getCurrent();
  for (auto& item : item_list)
  {
    auto mpos = Graphics::getMousePosition();
    if (item->bbox.check(mpos.x, mpos.y))
      set_focus(item);

    auto sloc = item->bbox.getLocate();
    if (item->draw_border)
      item->drawBG();
    if (item->on_edit)
    {
      item->drawCursor();
      if (TextInput::onInput())
        item->text = TextInput::get();
    }

    auto gloc = Graphics::calcLocate(sloc.x + BaseX, sloc.y + item->ofs_y);
    if (item->text.empty())
    {
      font->setColor(item->ph_color);
      font->print(item->place_holder.c_str(), gloc.x, gloc.y);
    }
    else
    {
      font->setColor(item->font_color);
      font->print(item->text.c_str(), gloc.x, gloc.y);
    }
  }
}

} // namespace TextBox
