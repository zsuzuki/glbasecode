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
  if (num == GLFW_KEY_LEFT)
  {
    if (manage->index != manage->buffer.begin())
      manage->index--;
  }
  else if (num == GLFW_KEY_RIGHT)
  {
    if (manage->index != manage->buffer.end())
      manage->index++;
  }
  else if (num == GLFW_KEY_BACKSPACE)
  {
    auto& buffer = manage->buffer;
    auto& index  = manage->index;
    if (index != buffer.begin())
      index = buffer.erase(--index);
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
