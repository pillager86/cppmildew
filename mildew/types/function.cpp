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

#include "function.hpp"

namespace mildew
{

    ScriptFunction::ScriptFunction(const std::string& fname, const NativeFunction& nfunc, bool is_class)
    : ScriptObject(is_class ? "Class" : "Function", nullptr), type_(Type::NATIVE_FUNCTION), function_name_(fname),
      closure_(nullptr), is_class_(is_class), is_generator_(false), native_function_(nfunc)
    {
        InitializePrototypeProperty();
    }

    ScriptFunction::ScriptFunction(const std::string& fname, const std::vector<std::string>& args,
        const std::vector<std::uint8_t>& bc, bool is_c, bool is_g)
    : ScriptObject(is_c? "Class": "Function", nullptr), type_(Type::SCRIPT_FUNCTION), function_name_(fname), 
      arg_names_(args), closure_(nullptr),
      is_class_(is_c), is_generator_(is_g), native_function_(nullptr), compiled_(bc)
    {
        InitializePrototypeProperty();
    }

    std::shared_ptr<ScriptFunction> ScriptFunction::Copy(const std::shared_ptr<Environment>& env) const
    {
        if(type_ == Type::SCRIPT_FUNCTION)
        {
            auto newFunc = std::make_shared<ScriptFunction>(function_name_, arg_names_, compiled_, is_class_,
                is_generator_);
            newFunc->closure_ = env;
            return newFunc;
        }
        else 
        {
            return std::make_shared<ScriptFunction>(function_name_, native_function_, is_class_);
        }
    }

    void ScriptFunction::Bind(const ScriptAny& this_obj)
    {
        bound_this_ = this_obj;
    }

    std::shared_ptr<ScriptFunction> ScriptFunction::BindCopy(const ScriptAny& this_obj) const 
    {
        auto newFunc = Copy(closure_);
        newFunc->Bind(this_obj);
        return newFunc;
    }

    size_t ScriptFunction::GetHash() const
    {
        // TODO improve
        auto hash = std::hash<std::string>()(function_name_);
        return hash;
    }

    bool ScriptFunction::IsInstanceOf(const std::shared_ptr<ScriptObject>& obj, 
        const std::shared_ptr<ScriptFunction>& clazz)
    {
        if(obj == nullptr || clazz == nullptr)
            return false;
        auto proto = obj->prototype();
        while(proto != nullptr)
        {
            auto ctor = proto->LookupField("constructor").ToValue<ScriptFunction>();
            if(ctor.get() == clazz.get())
                return true;
            proto = proto->prototype();
        }
        return false;
    }

    bool ScriptFunction::operator<(const ScriptFunction& func) const 
    {
        if(type_ != func.type_)
            return type_ < func.type_;
        // TODO proper implementation
        return function_name_ < func.function_name_;
    }

    bool ScriptFunction::operator==(const ScriptFunction& func) const
    {
        if(type_ != func.type_)
            return false;
        if(type_ == Type::SCRIPT_FUNCTION)
            return compiled_ == func.compiled_;
        else // TODO fix
            return &native_function_ == &func.native_function_;
    }

    void ScriptFunction::InitializePrototypeProperty()
    {
        auto proto = std::make_shared<ScriptObject>("Object", nullptr);
        (*proto)["constructor"] = ScriptAny(std::shared_ptr<ScriptFunction>(this));
        dictionary_["prototype"] = proto;
    }

    std::ostream& operator<<(std::ostream& os, const ScriptFunction& func)
    {
        os << func.name() << " " << func.function_name();
        return os;
    }

} // namespace mildew