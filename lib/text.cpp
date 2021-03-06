#include "text.h"
#include "codeconv.h"
#include "gl.h"
#include "pulldown.h"
#include <cctype>
#include <iostream>
#include <memory>

namespace TextInput
{
namespace
{
struct Manage
{
  Buffer&          buffer;
  Buffer::iterator index;
  size_t           max_length;
  Pulldown::ID     pulldown;
  InputStyle       input_style;
  Manage(Buffer& b, size_t m)
      : buffer(b), max_length(m), input_style(InputStyle::Text)
  {
    index = buffer.end();
  }

  // 文字列取得
  std::string getString() const
  {
    std::string r;
    r.reserve(buffer.size());
    r.resize(0);
    for (auto& c : buffer)
    {
      char b[4];
      int  cnt = CodeConv::U32ToU8(c, b);
      for (int i = 0; i < cnt; i++)
        r.push_back(b[i]);
    }
    return r;
  }

  // 入力候補更新
  void updatePulldown()
  {
    if (pulldown)
    {
      if (pulldown->setFilter(getString()) && pulldown->isOpened() == false)
      {
        // 入力の結果選択できる候補があるならウィンドウ表示
        if (buffer.size() > 0)
          pulldown->open();
      }
    }
  }
};
std::shared_ptr<Manage> manage;

//
void
addu8str(const char* text)
{
  auto&    buffer = manage->buffer;
  auto&    index  = manage->index;
  char32_t ch;
  while (int r = CodeConv::U8ToU32(text, ch))
  {
    if (ch == '\0')
      break;

    index = buffer.insert(index, ch);
    index++;
    text += r;
  }
}

//
void
key_input(int num, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS && action != GLFW_REPEAT)
    return;

  auto& buffer  = manage->buffer;
  auto& index   = manage->index;
  auto  forward = [&]() {
    if (index != buffer.end())
      index++;
  };
  auto backward = [&]() {
    if (index != buffer.begin())
      index--;
  };
  auto home = [&]() { index = buffer.begin(); };
  auto end  = [&]() { index = buffer.end(); };
  auto del  = [&]() {
    if (!buffer.empty() && index != buffer.end())
      index = buffer.erase(index);
  };
  auto linedel = [&]() {
    if (index != buffer.end())
      index = buffer.erase(index, buffer.end());
  };
  auto paste = [&]() { addu8str(Graphics::getClipboardString()); };
  auto copy  = [&](bool cut) {
    Graphics::setClipboardString(get().c_str());
    if (cut)
    {
      buffer.clear();
      index = buffer.begin();
    }
  };

  //
  auto sz = buffer.size();
  if (num == GLFW_KEY_LEFT)
    backward();
  else if (num == GLFW_KEY_RIGHT)
    forward();
  else if (num == GLFW_KEY_HOME)
    home();
  else if (num == GLFW_KEY_END)
    end();
  else if (num == GLFW_KEY_DELETE)
    (mods == GLFW_MOD_CONTROL) ? linedel() : del();
  else if (num == GLFW_KEY_BACKSPACE)
  {
    if (index != buffer.begin())
    {
      if (mods == GLFW_MOD_CONTROL || mods == GLFW_MOD_SHIFT)
        index = buffer.erase(buffer.begin(), index);
      else
        index = buffer.erase(--index);
    }
  }
  else if (mods == GLFW_MOD_CONTROL)
  {
    if (num == GLFW_KEY_A)
      home();
    else if (num == GLFW_KEY_E)
      end();
    else if (num == GLFW_KEY_F)
      forward();
    else if (num == GLFW_KEY_B)
      backward();
    else if (num == GLFW_KEY_D)
      del();
    else if (num == GLFW_KEY_K)
      linedel();
#if defined(_MSC_VER)
    else if (num == GLFW_KEY_V)
      paste();
    else if (num == GLFW_KEY_X)
      copy(true);
    else if (num == GLFW_KEY_C)
      copy(false);
#endif
  }
  else if (mods == GLFW_MOD_SUPER)
  {
#if !defined(_MSC_VER)
    if (num == GLFW_KEY_V)
      paste();
    else if (num == GLFW_KEY_X)
      copy(true);
    else if (num == GLFW_KEY_C)
      copy(false);
#endif
  }
  if (buffer.size() != sz)
    manage->updatePulldown();
}

//
bool
check_number(int code)
{
  auto& buff = manage->buffer;
  if (buff.empty() == false && buff[0] == '-')
  {
    // 先頭に既に符号が置かれていたら、その前には入力できない
    if (manage->index == buff.begin())
      return false;
  }
  // 数字は通過
  if (std::isdigit(code))
    return true;
  if (code == '.')
  {
    // 複数置けない
    for (auto c : manage->buffer)
      if (c == '.')
        return false;
    return true;
  }
  else if (code == '-')
  {
    // 符号は先頭に１つのみ
    return manage->index == buff.begin();
  }
  return false;
}

//
bool
check_style_code(int code)
{
  switch (manage->input_style)
  {
  case InputStyle::Text:
    // テキスト入力は全て通過
    break;
  case InputStyle::Number:
    // 数値チェック
    return check_number(code);
  }
  return true;
}

//
void
text_input(int code)
{
  auto& buff = manage->buffer;
  if (manage->max_length <= buff.size())
    return;

  if (check_style_code(code) == false)
    return;

  if (manage->index == buff.end())
  {
    buff.push_back(code);
    manage->index = buff.end();
  }
  else
  {
    manage->index = buff.insert(manage->index, code) + 1;
  }
  manage->updatePulldown();
}

} // namespace

//
void
start(Buffer& buff, size_t maxlength)
{
  if (manage)
    return;
  manage = std::make_shared<Manage>(buff, maxlength);
  Graphics::setTextInputCallback(key_input, text_input);
}

//
void
finish()
{
  if (!manage)
    return;
  Graphics::setTextInputCallback(nullptr, nullptr);
  auto& pd = manage->pulldown;
  if (pd && pd->isOpened())
    pd->close();
  manage.reset();
}

//
bool
onInput()
{
  return manage ? true : false;
}

//
size_t
getIndex()
{
  return std::distance(manage->buffer.begin(), manage->index);
}

//
void
setIndex(size_t p)
{
  auto& buffer = manage->buffer;
  auto  sz     = buffer.size();
  if (p >= sz)
    manage->index = buffer.end();
  else
    manage->index = std::next(manage->buffer.begin(), p);
}

//
CursorOffset
getIndexPos()
{
  CursorOffset ofs;
  auto         c = getIndex();
  ofs.left       = 21.0 * c;
  ofs.right      = ofs.left + 21.0;
  return ofs;
}

//
std::string
get()
{
  return manage ? manage->getString() : "";
}

//
void
setBuffer(Buffer& buff, std::string initial)
{
  auto     p = initial.c_str();
  char32_t ch;
  buff.resize(0);
  while (int r = CodeConv::U8ToU32(p, ch))
  {
    if (ch == '\0')
      break;

    p += r;
    buff.push_back(ch);
  }
}

//
void
setPulldown(Parts::IDPtr pd)
{
  auto new_pd      = std::dynamic_pointer_cast<Pulldown::Base>(pd);
  manage->pulldown = new_pd;
  if (new_pd)
  {
    new_pd->setChanged([](auto, auto s) {
      setBuffer(manage->buffer, s);
      setIndex(manage->max_length);
    });
    // if (new_pd->isOpened() == false)
    //   new_pd->open();
  }
}

//
void
setInputStyle(InputStyle st)
{
  manage->input_style = st;
}

} // namespace TextInput
