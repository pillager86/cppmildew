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
#include <string>
#include <unordered_map>

#include "types/any.hpp"

namespace mildew
{
    struct EnvEntry
    {
        bool is_const;
        ScriptAny value;
    };

    class Interpreter; // forward declaration
    
    class Environment
    {
        Environment(Interpreter* i); // global environment
        Environment(const std::shared_ptr<Environment>& par, const std::string n = "<environment>");

        bool DeclareVariable(const std::string& var_name, const ScriptAny& value, const bool is_const);
        size_t Depth() const;
        void ForceRemoveVariable(const std::string& var_name);
        void ForceSetVariable(const std::string& var_name, const ScriptAny& value, const bool is_const);
        Environment& G();
        EnvEntry* LookupVariable(const std::string& var_name);
        ScriptAny* ReassignVariable(const std::string& var_name, const ScriptAny& new_value, bool& failed_const);
        ScriptAny* ReassignVariable(const std::string& var_name, const ScriptAny& new_value);
        void UnsetVariable(const std::string& var_name);
        bool VariableExists(const std::string& var_name);

        std::shared_ptr<Environment> parent() const { return parent_; }
        std::string name() const { return name_; }
        Interpreter* interpreter();
    private:
        std::shared_ptr<Environment> parent_;
        std::string name_;
        std::unordered_map<std::string, EnvEntry> value_table_;
        Interpreter* interpreter_; // environments must never outlive host interpreter
    };
}

namespace std
{
    template<>
    struct hash<mildew::EnvEntry>
    {
        size_t operator()(const mildew::EnvEntry& entry) const
        {
            return entry.value.GetHash();
        }
    };
}