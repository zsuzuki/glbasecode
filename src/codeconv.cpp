//
// 以下のサイトから頂いたコードを使用
// https://qiita.com/benikabocha/items/e943deb299d0f816f161
//
#include <array>
#include <cstdint>
#include <string>

#include "codeconv.h"

namespace CodeConv
{
namespace
{
int
GetU8ByteCount(char ch)
{
  if (0 <= uint8_t(ch) && uint8_t(ch) < 0x80)
  {
    return 1;
  }
  if (0xC2 <= uint8_t(ch) && uint8_t(ch) < 0xE0)
  {
    return 2;
  }
  if (0xE0 <= uint8_t(ch) && uint8_t(ch) < 0xF0)
  {
    return 3;
  }
  if (0xF0 <= uint8_t(ch) && uint8_t(ch) < 0xF8)
  {
    return 4;
  }
  return 0;
}

bool
IsU8LaterByte(char ch)
{
  return 0x80 <= uint8_t(ch) && uint8_t(ch) < 0xC0;
}
} // namespace

int
U8ToU32(const char* msg, char32_t& u32Ch)
{
  int numBytes = GetU8ByteCount(*msg);
  if (numBytes == 0)
    return 0;

  const char* u8Ch = msg;
  switch (numBytes)
  {
  case 1:
    u32Ch = char32_t(uint8_t(u8Ch[0]));
    break;
  case 2:
    if (!IsU8LaterByte(u8Ch[1]))
    {
      return 0;
    }
    if ((uint8_t(u8Ch[0]) & 0x1E) == 0)
    {
      return 0;
    }

    u32Ch = char32_t(u8Ch[0] & 0x1F) << 6;
    u32Ch |= char32_t(u8Ch[1] & 0x3F);
    break;
  case 3:
    if (!IsU8LaterByte(u8Ch[1]) || !IsU8LaterByte(u8Ch[2]))
    {
      return 0;
    }
    if ((uint8_t(u8Ch[0]) & 0x0F) == 0 && (uint8_t(u8Ch[1]) & 0x20) == 0)
    {
      return 0;
    }

    u32Ch = char32_t(u8Ch[0] & 0x0F) << 12;
    u32Ch |= char32_t(u8Ch[1] & 0x3F) << 6;
    u32Ch |= char32_t(u8Ch[2] & 0x3F);
    break;
  case 4:
    if (!IsU8LaterByte(u8Ch[1]) || !IsU8LaterByte(u8Ch[2]) ||
        !IsU8LaterByte(u8Ch[3]))
    {
      return 0;
    }
    if ((uint8_t(u8Ch[0]) & 0x07) == 0 && (uint8_t(u8Ch[1]) & 0x30) == 0)
    {
      return 0;
    }

    u32Ch = char32_t(u8Ch[0] & 0x07) << 18;
    u32Ch |= char32_t(u8Ch[1] & 0x3F) << 12;
    u32Ch |= char32_t(u8Ch[2] & 0x3F) << 6;
    u32Ch |= char32_t(u8Ch[3] & 0x3F);
    break;
  default:
    return 0;
  }

  return numBytes;
}
} // namespace CodeConv