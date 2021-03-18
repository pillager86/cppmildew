#include "utf.h"

#include <codecvt>
#include <locale>

namespace mildew
{
    std::string EncodeChar32(const char32_t dc)
    {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.to_bytes(std::u32string( {dc} ));
    }
}