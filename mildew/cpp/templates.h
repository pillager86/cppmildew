#pragma once
#include <type_traits>

namespace mildew
{
    template<typename T>
    struct dependent_false : std::false_type {};
}