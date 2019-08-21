#include "pulldown.h"
#include "bb.h"
#include "codeconv.h"
#include "gl.h"
#include "layer.h"
#include "primitive2d.h"
#include <iostream>
#include <list>

namespace Pulldown
{
namespace
{
FontDraw::WidgetPtr font;
// 選択用アイテム
struct SelItem
{
  size_t             index;
  const std::string* str;
};

//
struct Item : public Base
{
  using BBox     = BoundingBox::Rect;
  using Parent   = const Parts::ID;
  using SelItems = std::vector<SelItem>;

  double      x, y;
  double      width, height;
  Parent*     parent;
  std::string filter;
  int         select_index;
  int         mouse_focus;
  bool        opened;
  bool        open_request;
  bool        close_request;
  BBox        bbox;
  List        items;
  size_t      nb_disp;
  size_t      disp_top;
  Selected    selected_func;
  Selected    changed_func;
  SelItems    sel_items;

  ~Item() override = default;
  double getX() const override { return x; }
  double getY() const override { return y; }
  int    getWidth() const override { return width; }
  int    getHeight() const override { return height; }
  void   setParent(const Parts::ID* p) override { parent = p; }
  bool   setFilter(std::string f) override;
  size_t getIndex() const override { return select_index; }
  void   open() override;
  void   close() override;
  bool   isOpened() const override { return opened || open_request; }
  void   setSelected(Selected sf) override { selected_func = sf; }
  void   setChanged(Selected sf) override { changed_func = sf; }

  operator bool() const { return isOpened(); }

  bool updateAndDraw(const Graphics::Locate&);
  void key_input();
  bool do_open();
  bool do_close();
  void update_list();
  void selected();
  void changed(int);
};
using ItemPtr  = std::shared_ptr<Item>;
using ClickAct = Graphics::ClickCallback::Action;
Layer<Item> layer;

ItemPtr focus_item; // マウスがフォーカスしているプルダウン
ItemPtr open_item;  // 現在オープンしているプルダウン

//
void
on_click(ClickAct action, bool enter)
{
  if (action == ClickAct::Press)
  {
    if (focus_item)
    {
      int mf = focus_item->mouse_focus;
      if (mf != -1)
      {
        focus_item->selected();
        focus_item->close();
      }
    }
    else if (open_item && open_item->parent->getFocus() == false)
      open_item->close();
  }
}

//
void
print(const std::string& msg, double x, double y)
{
  auto loc = Graphics::calcLocate(x, y);
  font->print(msg.c_str(), (float)loc.x, (float)loc.y);
}

// 洗濯したアイテムを決定
void
Item::selected()
{
  if (selected_func)
  {
    auto& item = sel_items[select_index];
    selected_func(item.index, *item.str);
  }
}

// フォーカス変更したアイテムを通知
void
Item::changed(int idx)
{
  if (idx != select_index && changed_func)
  {
    auto& item = sel_items[select_index];
    changed_func(item.index, *item.str);
  }
}

// 表示する候補の絞り込み
void
Item::update_list()
{
  sel_items.resize(0);
  for (size_t i = 0; i < items.size(); i++)
  {
    const auto& str = items[i];
    if (filter.empty() || str.find(filter) != std::string::npos)
      sel_items.push_back({i, &str});
  }
  select_index = disp_top;
}

// フィルター更新
bool
Item::setFilter(std::string f)
{
  if (filter != f)
  {
    filter = f;
    update_list();
  }
  // 選択できる候補が無いなら閉じてしまう
  bool valid = sel_items.size() > 0;
  if (valid == false && isOpened())
    close();
  return valid;
}

// 実際のオープン処理
bool
Item::do_open()
{
  if (!open_request)
    return false;

  if (open_item)
  {
    auto p = open_item;
    p->close();
  }

  opened       = true;
  open_request = false;
  if (parent)
  {
    x = parent->getX() + 20;
    y = parent->getY() + parent->getHeight();
  }
  bbox = BoundingBox::Rect{x, y, width, height};

  // オープン時に最初の候補の選択を通知
  changed(-1);

  return true;
}

//
bool
Item::do_close()
{
  if (!close_request)
    return false;

  open_item     = ItemPtr();
  opened        = false;
  open_request  = false;
  close_request = false;

  return true;
}

//
void
Item::close()
{
  // リクエストのみ
  open_request  = false;
  close_request = true;
}

//
void
Item::open()
{
  // リクエストのみ
  open_request  = true;
  close_request = false;
}

//
void
Item::key_input()
{
  auto& key = Graphics::getKeyInput();
  auto  sz  = sel_items.size();
  switch (key.getRepeat())
  {
  case Key::Code::Up:
    select_index -= select_index > 0;
    break;
  case Key::Code::Down:
    select_index += select_index < sz - 1;
    break;
  case Key::Code::Tab:
    select_index = (select_index + (key.onShift() ? sz - 1 : 1)) % sz;
    break;
  case Key::Code::Esc:
    close();
    break;
  case Key::Code::Enter:
    selected();
    if (parent->getFocus() == false)
      close();
    break;
  default:
    break;
  }
  if (select_index >= nb_disp + disp_top)
    disp_top = select_index - nb_disp + 1;
  else if (select_index < disp_top)
    disp_top = select_index;
}

//
bool
Item::updateAndDraw(const Graphics::Locate& mpos)
{
  if (opened == false)
    return false;

  auto depth = parent->getDepth() - 0.1f;

  // 下敷きを描画
  auto bcol = Graphics::DarkGray;
  auto loc  = bbox.getLocate();
  auto btm  = bbox.getBottom();
  Primitive2D::setDepth(depth - 0.1f);
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, bcol, true);
  auto lcol = Graphics::White;
  Primitive2D::setDepth(depth - 0.11f);
  Primitive2D::drawBox(loc.x, loc.y, btm.x, btm.y, lcol, false);

  // フォーカス
  int mf  = -1;
  int sel = select_index;
  if (bbox.check(mpos.x, mpos.y))
  {
    int oy = (int)(mpos.y - loc.y - 8.0) / 42;
    if (oy >= 0 && oy < nb_disp)
    {
      mf = oy;
      if (mouse_focus != mf)
      {
        // フォーカスしたところにちゃんとアイテムがあるなら移動
        int idx = mf + disp_top;
        if (idx < sel_items.size())
          select_index = idx;
        else
          mf = -1;
      }
    }
  }
  mouse_focus = mf;
  key_input();
  changed(sel);

  // 選択
  {
    double y    = loc.y + (select_index - disp_top) * 42.0 + 8.0;
    auto   fcol = Graphics::LightGray;
    Primitive2D::setDepth(depth - 0.12f);
    Primitive2D::drawBox(loc.x + 5, y, btm.x - 5, y + 42, fcol, true);
  }

  // 文字列描画
  font->setDepth(depth - 0.13f);
  int nb_d = nb_disp > sel_items.size() ? sel_items.size() : nb_disp;
  for (int i = 0; i < nb_d; i++)
  {
    auto& str  = *sel_items[i + disp_top].str;
    auto  fcol = Graphics::White;
    font->setColor(fcol);
    print(str, getX() + 20, getY() + i * 42.0 + 42.0);
  }

  return mf != -1;
}

} // namespace

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

  // オープンリクエスト受付
  auto& item_list = layer.getCurrent();
  bool  exist     = false;
  for (auto& i : item_list)
  {
    if (i->do_open())
    {
      open_item = i;
      exist     = true;
      break;
    }
    if (open_item == i)
      exist = true;
    i->do_close();
  }

  // オープンしているプルダウンの処理(画面上に１つだけ)
  focus_item = ItemPtr{};
  if (open_item)
  {
    if (exist)
    {
      auto mpos = Graphics::getMousePosition();
      if (open_item->updateAndDraw(mpos))
        focus_item = open_item;
    }
    else
      open_item->close();
  }

  Primitive2D::popDepth();
  font->popDepth();
}

//
ID
create(List&& l, size_t nb_disp)
{
  auto item          = std::make_shared<Item>();
  item->x            = 0.0;
  item->y            = 0.0;
  item->parent       = nullptr;
  item->filter       = "";
  item->select_index = 0;
  item->opened       = false;
  item->mouse_focus  = -1;

  int ml = 0;
  for (auto& s : l)
  {
    auto len = CodeConv::U8Length2(s.c_str()) * 21.0;
    if (len > ml)
      ml = len;
  }
  size_t nd = l.size();
  item->sel_items.reserve(nd);
  if (nd > nb_disp)
    nd = nb_disp;
  item->width    = ml + 20 + 20;
  item->height   = nd * 42.0 + 20;
  item->nb_disp  = nd;
  item->disp_top = 0;
  item->items    = l;
  item->update_list();

  auto& item_list = layer.getCurrent();
  item_list.push_back(item);

  return item;
}

//
void
erase(ID i)
{
  auto ip = std::dynamic_pointer_cast<Item>(i);
  if (ip)
    layer.erase(ip);
}

// レイヤー切り替え
void
bindLayer(std::string l)
{
  if (layer.bind(l))
    focus_item.reset();
}

//
void
clearLayer(std::string l)
{
  layer.clear(l);
  focus_item.reset();
}

} // namespace Pulldown