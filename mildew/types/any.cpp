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
#include "any.h"

#include "object.h"

namespace mildew
{
    ScriptAny::ScriptAny(const ScriptAny& other)
    {
        switch(other.type_)
        {
        case Type::UNDEFINED:
        case Type::NULL_:
            break;
        case Type::BOOLEAN:
            SetValue(other.as_boolean_);
            break;
        case Type::INTEGER:
            SetValue(other.as_integer_);
            break;
        case Type::DOUBLE:
            SetValue(other.as_double_);
            break;
        case Type::OBJECT:
            SetValue(other.as_object_);
            break;
        }
        type_ = other.type_;
    }

    ScriptAny::~ScriptAny()
    {
        DestructObject();
    }

    bool ScriptAny::operator==(const ScriptAny& other) const
    {
        if(type_ == Type::UNDEFINED && other.type_ == Type::UNDEFINED)
            return true;
        else if((type_ == Type::UNDEFINED || type_ == Type::NULL_) &&
          (other.type_ == Type::UNDEFINED || other.type_ == Type::NULL_))
            return true;
        else if(type_ == Type::UNDEFINED || other.type_ == Type::UNDEFINED)
            return false;

        if(type_ == Type::NULL_ || other.type_ == Type::NULL_)
            return false;

        // TODO string
        
        if(IsNumber() && other.IsNumber())
        {
            if(type_ == Type::DOUBLE || other.type_ == Type::DOUBLE)
                return ToValue<double>() == other.ToValue<double>();
            return ToValue<std::int64_t>() == other.ToValue<std::int64_t>();
        }

        // TODO array

        // TODO function

        if(type_ != other.type_)
            return false;

        return *as_object_ == *(other.as_object_);
    }

    bool ScriptAny::operator<(const ScriptAny& other) const
    {
        if(type_ == Type::UNDEFINED)
            return other.type_ >= Type::UNDEFINED;
        if(type_ == Type::NULL_)
            return other.type_ >= Type::NULL_;
        if(IsNumber() && !other.IsNumber())
            return true;
        else if(!IsNumber() && other.IsNumber())
            return false;
        else if(IsNumber() && other.IsNumber())
            return ToValue<double>() < other.ToValue<double>();
        // handle string
        // handle array
        // handle function
        if(IsObject() && other.IsObject())
            return *as_object_ < *(other.as_object_);

        if(type_ != other.type_)
            return type_ < other.type_;
        
        return false;
    }

    size_t ScriptAny::GetHash() const
    {
        switch(type_)
        {
        case Type::UNDEFINED: return -1;
        case Type::NULL_: return 0;
        case Type::BOOLEAN: return std::hash<bool>()(as_boolean_);
        case Type::INTEGER: return std::hash<std::int64_t>()(as_integer_);
        case Type::DOUBLE: return std::hash<double>()(as_double_);
        case Type::OBJECT: return as_object_->GetHash();
        }
        return -1;
    }

    bool ScriptAny::IsInteger() const 
    {
        return type_ == Type::NULL_ || type_ == Type::BOOLEAN || type_ == Type::INTEGER;
    }

    bool ScriptAny::IsNumber() const
    {
        return type_ == Type::DOUBLE || IsInteger();
    }

    bool ScriptAny::IsObject() const
    {
        return type_ == Type::OBJECT; // more later
    }

    void ScriptAny::DestructObject()
    {
        if(IsObject())
            as_object_.~shared_ptr<ScriptObject>();
    }

    std::ostream& operator<<(std::ostream& os, const ScriptAny& any)
    {
        switch(any.type())
        {
            case ScriptAny::Type::UNDEFINED:
                os << "undefined";
                break;
            case ScriptAny::Type::NULL_:
                os << "null";
                break;
            case ScriptAny::Type::BOOLEAN:
                os << (any.as_boolean_ ? "true" : "false");
                break;
            case ScriptAny::Type::INTEGER:
                os << any.as_integer_;
                break;
            case ScriptAny::Type::DOUBLE:
                os << any.as_double_;
                break;
            case ScriptAny::Type::OBJECT:
                os << any.as_object_;
                break;
            default:
                os << "<Unknown ScriptAny type>";
                break;
        }
        return os;
    }
}