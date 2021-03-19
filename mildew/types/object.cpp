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
#include "object.h"

#include <sstream>

namespace mildew
{
    ScriptObject::ScriptObject(const std::string& type, std::shared_ptr<ScriptObject> proto, Object* native)
    : name_(type), prototype_(proto), native_object_(native)
    {
        // todo: get object prototype

    }

    ScriptObject::ScriptObject(const std::string& type)
    : name_(type), prototype_(nullptr), native_object_(nullptr)
    {

    }

    ScriptObject::~ScriptObject()
    {
        if(native_object_)
            delete native_object_;
    }

    void ScriptObject::native_object(Object* obj)
    {
        if(native_object_)
            delete native_object_;
        native_object_ = obj;
    }

    void ScriptObject::AssignField(const std::string& name, const ScriptAny& value)
    {
        // TODO check __proto__ and __super__
        dictionary_.emplace(name, value);
    }

    size_t ScriptObject::GetHash() const
    {
        // TODO something better
        return dictionary_.size();
    }

    ScriptAny ScriptObject::LookupField(const std::string& name) const
    {
        // TODO check __proto__
        // TODO check __super__
        if(dictionary_.count(name) > 0)
            return dictionary_.at(name);
        if(prototype_.get() != nullptr)
            return prototype_->LookupField(name);
        return ScriptAny();
    }

    bool ScriptObject::operator<(const ScriptObject& other) const
    {
        // TODO, check keys and prototype
        return dictionary_.size() < other.dictionary_.size();
    }

    bool ScriptObject::operator==(const ScriptObject& other) const
    {
        // TODO: compare getters and setters once implemented
        return dictionary_ == other.dictionary_ && prototype_ == other.prototype_;
    } 

    ScriptAny& ScriptObject::operator[](const std::string& index)
    {
        return dictionary_[index];
    }

    std::string ScriptObject::FormattedString() const
    {
        std::stringstream ss;
        ss << "{";
        size_t counter = 0;
        for(const auto& [key, value] : dictionary_)
        {
            ss << '"' << key << "\": ";
            if(value.type() == ScriptAny::Type::OBJECT)
                ss << value.ToValue<ScriptObject>()->FormattedString();
            else 
                ss << value;
            if(counter < dictionary_.size() - 1)
                ss << ", ";
            ++counter;
        }
        ss << "}";
        return ss.str();
    }

    std::ostream& operator<<(std::ostream& os, const ScriptObject& obj)
    {
        // TODO configuration option for formatted string
        if(obj.native_object())
            os << "[Native object " << obj.native_object() << "]";
        else 
            os << "[" << obj.name() << " " << &obj << "]";
        return os;
    }
}