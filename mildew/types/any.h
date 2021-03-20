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
#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <type_traits>

#include "../../cpp/templates.h"

namespace mildew
{
    class ScriptObject;

    template<typename T>
    struct ConvertReturn
    {
        using type = T;
    };

    template<>
    struct ConvertReturn<ScriptObject>
    {
        using type = std::shared_ptr<ScriptObject>;
    };

    struct ScriptAny final
    {
        enum class Type { UNDEFINED, NULL_, BOOLEAN, INTEGER, DOUBLE, OBJECT };

        ScriptAny() {}
        ScriptAny(const ScriptAny& );
        ~ScriptAny();

        template<typename T>
        ScriptAny(const T& value)
        {
            SetValue(value);
        }

        bool operator==(const ScriptAny& other) const;
        bool operator<(const ScriptAny& other) const;

        template<typename T>
        ScriptAny& operator=(const T& value)
        {
            SetValue(value);
            return *this;
        }

        Type type() const { return type_; }

        size_t GetHash() const;

        bool IsInteger() const;
        bool IsNumber() const;
        bool IsObject() const;

        template<typename T>
        typename ConvertReturn<T>::type ToValue() const
        {
            if constexpr(std::is_same_v<bool, std::remove_cv_t<T>>)
            {
                switch(type_)
                {
                case Type::NULL_: case Type::UNDEFINED: return false;
                case Type::BOOLEAN: return as_boolean_;
                case Type::INTEGER: return as_integer_ != 0;
                case Type::DOUBLE: return as_double_ != 0.0;
                // TODO helper functions for string and array and function
                case Type::OBJECT: return as_object_.get() != nullptr;
                }
                return static_cast<T>(false);
            }
            else if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
            {
                switch(type_)
                {
                case Type::NULL_: case Type::UNDEFINED: return static_cast<T>(0);
                case Type::BOOLEAN: return static_cast<T>(as_boolean_);
                case Type::INTEGER: return static_cast<T>(as_integer_);
                case Type::DOUBLE: return static_cast<T>(as_double_);
                case Type::OBJECT: return static_cast<T>(0);
                }
                return static_cast<T>(0);
            }
            else if constexpr(std::is_same_v<ScriptObject, std::remove_cv_t<T>>)
            {
                if(IsObject())
                    return as_object_;
                else 
                    return std::shared_ptr<ScriptObject>(nullptr);
            }
            else 
            {
                static_assert(cpp::dependent_false<T>::value, "Unable to convert type");
            }
        }

    private:
        template<typename T>
        void SetValue(const T& value)
        {
            if constexpr(std::is_same_v<decltype(nullptr), std::remove_cv_t<T>>)
            {
                DestructObject();
                type_ = Type::NULL_;
            }
            else if constexpr(std::is_same_v<bool, std::remove_cv_t<T>>)
            {
                DestructObject();
                type_ = Type::BOOLEAN;
                as_boolean_ = value;
            }
            else if constexpr(std::is_integral_v<T>)
            {
                DestructObject();
                type_ = Type::INTEGER;
                as_integer_ = value;
            }
            else if constexpr(std::is_floating_point_v<T>)
            {
                DestructObject();
                type_ = Type::DOUBLE;
                as_double_ = value;
            }
            else if constexpr(std::is_same_v<std::shared_ptr<ScriptObject>, std::remove_cv_t<T>>)
            {
                if(IsObject())
                {
                    as_object_ = value;
                }
                else 
                {
                    new (&as_object_) std::shared_ptr<ScriptObject>(value);
                }
                type_ = Type::OBJECT;
            }
            else
            {
                static_assert(cpp::dependent_false<T>::value, "Invalid type specified");
            }
        }

        void DestructObject();

        Type type_ = Type::UNDEFINED;
        union 
        {
            bool as_boolean_;
            std::int64_t as_integer_;
            double as_double_;
            // Also stores function, string, and array
            std::shared_ptr<ScriptObject> as_object_;
        };

        friend std::ostream& operator<<(std::ostream& os, const ScriptAny& any);
    };
} // namespace mildew

namespace std
{
    template<>
    struct hash<mildew::ScriptAny>
    {
        size_t operator()(const mildew::ScriptAny& obj) const
        {
            return obj.GetHash();
        }
    };
}