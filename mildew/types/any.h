#pragma once

#include <cstdint>
#include <ostream>
#include <type_traits>
#include "../cpp/templates.h"

namespace mildew
{
    struct ScriptAny
    {
        enum class Type { UNDEFINED, NULL_, BOOLEAN, INTEGER, DOUBLE };

        ScriptAny() {}

        template<typename T>
        ScriptAny(const T& value)
        {
            SetValue(value);
        }

        template<typename T>
        ScriptAny& operator=(const T& value)
        {
            SetValue(value);
            return *this;
        }

        Type type() const { return type_; }

    private:
        template<typename T>
        void SetValue(const T& value)
        {
            if constexpr(std::is_same_v<decltype(nullptr), std::remove_cv_t<T>>)
            {
                type_ = Type::NULL_;
            }
            else if constexpr(std::is_same_v<bool, std::remove_cv_t<T>>)
            {
                type_ = Type::BOOLEAN;
                as_boolean_ = value;
            }
            else if constexpr(std::is_integral_v<T>)
            {
                type_ = Type::INTEGER;
                as_integer_ = value;
            }
            else if constexpr(std::is_floating_point_v<T>)
            {
                type_ = Type::DOUBLE;
                as_double_ = value;
            }
            else
            {
                static_assert(dependent_false<T>::value, "Invalid type specified");
            }
        }

        Type type_ = Type::UNDEFINED;
        union 
        {
            bool as_boolean_;
            std::int64_t as_integer_;
            double as_double_;
        };

        friend std::ostream& operator<<(std::ostream& os, const ScriptAny& any);
    };
}