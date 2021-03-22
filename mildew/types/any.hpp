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
#include <string>
#include <type_traits>

#include "../../cppd/templates.hpp"
#include "../../cppd/utf8string.hpp"

namespace mildew
{
    class ScriptObject;
    class ScriptArray;
    class ScriptFunction;
    class ScriptString;

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

    template<>
    struct ConvertReturn<ScriptArray>
    {
        using type = std::shared_ptr<ScriptArray>;
    };

    template<>
    struct ConvertReturn<ScriptFunction>
    {
        using type = std::shared_ptr<ScriptFunction>;
    };

    template<>
    struct ConvertReturn<ScriptString>
    {
        using type = std::shared_ptr<ScriptString>;
    };

    struct ScriptAny final
    {
        enum class Type { UNDEFINED, NULL_, BOOLEAN, INTEGER, DOUBLE, OBJECT, ARRAY, FUNCTION, STRING };

        ScriptAny() {}
        ScriptAny(const ScriptAny& );
        ScriptAny(const std::string& str);
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

        ScriptAny& operator=(const ScriptAny& value)
        {
            SetValue(value);
            return *this;
        }

        ScriptAny& operator=(const std::string& str);

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
                case Type::OBJECT:
                case Type::ARRAY:
                case Type::FUNCTION:
                case Type::STRING:
                    return as_object_.get() != nullptr;
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
                case Type::OBJECT: 
                case Type::ARRAY:
                case Type::FUNCTION:
                case Type::STRING:
                    return static_cast<T>(0);
                }
                return static_cast<T>(0);
            }
            else if constexpr(std::is_same_v<ScriptObject, std::remove_cv_t<T>>)
            {
                if(IsObject())
                    return as_object_;
                else 
                    return std::shared_ptr<T>(nullptr);
            }
            else if constexpr(std::is_same_v<ScriptArray, std::remove_cv_t<T>>)
            {
                if(type_ == Type::ARRAY)
                    return std::dynamic_pointer_cast<T>(as_object_);
                else 
                    return std::shared_ptr<T>(nullptr);
            }
            else if constexpr(std::is_same_v<ScriptFunction, std::remove_cv_t<T>>)
            {
                if(type_ == Type::FUNCTION)
                    return std::dynamic_pointer_cast<T>(as_object_);
                else 
                    return std::shared_ptr<T>(nullptr);
            }
            else if constexpr(std::is_same_v<ScriptString, std::remove_cv_t<T>>)
            {
                if(type_ == Type::STRING)
                    return std::dynamic_pointer_cast<T>(as_object_);
                else 
                    return std::shared_ptr<T>(nullptr);
            }
            else 
            {
                static_assert(cppd::dependent_false<T>::value, "Unable to convert type");
            }
        }

        std::string ToString() const;
        cppd::UTF8String ToUTF8String() const;

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
            else if constexpr(std::is_same_v<std::shared_ptr<ScriptArray>, std::remove_cv_t<T>>)
            {
                if(IsObject())
                    as_object_ = value;
                else 
                    new (&as_object_) std::shared_ptr<ScriptArray>(value);
                type_ = Type::ARRAY;
            }
            else if constexpr(std::is_same_v<std::shared_ptr<ScriptFunction>, std::remove_cv_t<T>>)
            {
                if(IsObject())
                    as_object_ = value;
                else 
                    new (&as_object_) std::shared_ptr<ScriptFunction>(value);
                type_ = Type::FUNCTION;
            }
            else if constexpr(std::is_same_v<std::shared_ptr<ScriptString>, std::remove_cv_t<T>>)
            {
                if(IsObject())
                    as_object_ = value;
                else 
                    new (&as_object_) std::shared_ptr<ScriptString>(value);
                type_ = Type::STRING;
            }
            else
            {
                static_assert(cppd::dependent_false<T>::value, "Invalid type specified");
            }
        }

        void SetValue(const ScriptAny& value)
        {
            if(this == &value)
                return;
            switch(value.type_)
            {
            case Type::UNDEFINED: DestructObject(); break;
            case Type::NULL_: DestructObject(); break;
            case Type::BOOLEAN: DestructObject(); as_boolean_ = value.as_boolean_; break;
            case Type::INTEGER: DestructObject(); as_integer_ = value.as_integer_; break;
            case Type::DOUBLE: DestructObject(); as_double_ = value.as_double_; break;
            case Type::OBJECT: 
            case Type::ARRAY:
            case Type::FUNCTION:
            case Type::STRING:
                if(IsObject())
                    as_object_ = value.as_object_;
                else 
                    new (&as_object_) std::shared_ptr<ScriptObject>(value.as_object_);
                break;
            }
            type_ = value.type_;
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