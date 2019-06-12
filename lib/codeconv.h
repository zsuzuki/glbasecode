#pragma once

namespace CodeConv
{
// utf8 -> unicode32
int U8ToU32(const char* msg, char32_t& u32Ch);
// unicode32 -> utf8
int U32ToU8(const char32_t u32Ch, char* u8Ch);
// utf8 length
int U8Length(const char* msg);
// utf8 length(for font)
double U8Length2(const char* msg);
} // namespace CodeConv
