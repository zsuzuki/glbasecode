#include "text.h"
#include "codeconv.h"
#include "gl.h"
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
  Manage(Buffer& b, size_t m) : buffer(b), max_length(m)
  {
    index = buffer.end();
  }
};
std::shared_ptr<Manage> manage;

//
void
key_input(int num, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS)
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
  }
}

//
void
text_input(int code)
{
  auto& buff = manage->buffer;
  if (manage->max_length <= buff.size())
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
  if (!manage)
    return "";
  std::string r;
  r.reserve(manage->buffer.size());
  r.resize(0);
  for (auto& c : manage->buffer)
  {
    char b[4];
    int  cnt = CodeConv::U32ToU8(c, b);
    for (int i = 0; i < cnt; i++)
      r.push_back(b[i]);
  }
  return r;
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

} // namespace TextInput
