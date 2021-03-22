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

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>

#include "../../cppd/object.hpp"
#include "any.hpp"

namespace mildew
{
    class ScriptObject
    {
    public:
        ScriptObject(const std::string& type, std::shared_ptr<ScriptObject> proto, cppd::Object* native = nullptr);
        ScriptObject(const std::string& type);
        ScriptObject(const ScriptObject&) = delete;
        virtual ~ScriptObject();
        ScriptObject& operator=(const ScriptObject&) = delete;

        std::unordered_map<std::string, ScriptAny>& dictionary() { return dictionary_; }
        const std::string& name() const { return name_; }
        std::shared_ptr<ScriptObject> prototype() { return prototype_; }
        void prototype(std::shared_ptr<ScriptObject> proto) { prototype_ = proto; }
        cppd::Object* native_object() const { return native_object_; }
        void native_object(cppd::Object* obj);

        void AssignField(const std::string& name, const ScriptAny& value);
        virtual size_t GetHash() const;
        ScriptAny LookupField(const std::string& name) const;

        bool operator<(const ScriptObject& other) const;
        bool operator==(const ScriptObject& other) const;
        ScriptAny& operator[](const std::string& index);

    protected:
        std::unordered_map<std::string, ScriptAny> dictionary_;
        // std::unordered_map<std::string, std::shared_ptr<ScriptFunction>> getters_;
        // std::unordered_map<std::string, std::shared_ptr<ScriptFunction>> setters_;
    
    private:
        std::string FormattedString() const;
        
        std::string name_;
        std::shared_ptr<ScriptObject> prototype_;
        cppd::Object* native_object_;
    };

    std::ostream& operator<<(std::ostream& os, const ScriptObject& obj);
} // namespace mildew
