#include <cstdint>
#include <array>
#include <string>

int GetU8ByteCount(char ch) {
    if (0 <= uint8_t(ch) && uint8_t(ch) < 0x80) {
        return 1;
    }
    if (0xC2 <= uint8_t(ch) && uint8_t(ch) < 0xE0) {
        return 2;
    }
    if (0xE0 <= uint8_t(ch) && uint8_t(ch) < 0xF0) {
        return 3;
    }
    if (0xF0 <= uint8_t(ch) && uint8_t(ch) < 0xF8) {
        return 4;
    }
    return 0;
}

bool IsU8LaterByte(char ch) {
    return 0x80 <= uint8_t(ch) && uint8_t(ch) < 0xC0;
}

bool ConvChU8ToU32(const std::array<char, 4>& u8Ch, char32_t& u32Ch) {
    int numBytes = GetU8ByteCount(u8Ch[0]);
    if (numBytes == 0) {
        return false;
    }
    switch (numBytes) {
        case 1:
            u32Ch = char32_t(uint8_t(u8Ch[0]));
            break;
        case 2:
            if (!IsU8LaterByte(u8Ch[1])) {
                return false;
            }
            if ((uint8_t(u8Ch[0]) & 0x1E) == 0) {
                return false;
            }

            u32Ch = char32_t(u8Ch[0] & 0x1F) << 6;
            u32Ch |= char32_t(u8Ch[1] & 0x3F);
            break;
        case 3:
            if (!IsU8LaterByte(u8Ch[1]) || !IsU8LaterByte(u8Ch[2])) {
                return false;
            }
            if ((uint8_t(u8Ch[0]) & 0x0F) == 0 &&
                (uint8_t(u8Ch[1]) & 0x20) == 0) {
                return false;
            }

            u32Ch = char32_t(u8Ch[0] & 0x0F) << 12;
            u32Ch |= char32_t(u8Ch[1] & 0x3F) << 6;
            u32Ch |= char32_t(u8Ch[2] & 0x3F);
            break;
        case 4:
            if (!IsU8LaterByte(u8Ch[1]) || !IsU8LaterByte(u8Ch[2]) ||
                !IsU8LaterByte(u8Ch[3])) {
                return false;
            }
            if ((uint8_t(u8Ch[0]) & 0x07) == 0 &&
                (uint8_t(u8Ch[1]) & 0x30) == 0) {
                return false;
            }

            u32Ch = char32_t(u8Ch[0] & 0x07) << 18;
            u32Ch |= char32_t(u8Ch[1] & 0x3F) << 12;
            u32Ch |= char32_t(u8Ch[2] & 0x3F) << 6;
            u32Ch |= char32_t(u8Ch[3] & 0x3F);
            break;
        default:
            return false;
    }

    return true;
}

std::u32string
u8to32(std::string str)
{
    for (char c : str) {

    }
}