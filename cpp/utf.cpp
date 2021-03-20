/*
Copyright (C) 2021 pillager86.rf.gd

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 3 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "utf.h"

#include <codecvt>
#include <locale>

namespace cpp
{
    /**
     * Encodes a 4-byte character as a utf8 string
     * @param dc A 4-byte unicode character
     * @returns A standard string with the encoded character
     */
    std::string EncodeChar32(const char32_t dc)
    {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.to_bytes(std::u32string( {dc} ));
    }
}