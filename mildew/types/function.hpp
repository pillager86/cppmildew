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

#include <cstdint>
#include <functional>
#include <ostream>
#include <string>
#include <vector>

#include "../environment.hpp"
#include "any.hpp"
#include "object.hpp"

namespace mildew
{
    enum class NativeFunctionError
    {
        NO_ERROR = 0, 
        WRONG_NUMBER_OF_ARGS,
        WRONG_TYPE_OF_ARG,
        RETURN_VALUE_IS_EXCEPTION
    };

    using NativeFunction = std::function<ScriptAny(Environment&, ScriptAny&, 
        const std::vector<ScriptAny>&, NativeFunctionError&)>;

    class ScriptFunction : public ScriptObject
    {
    public:
        enum class Type { SCRIPT_FUNCTION, NATIVE_FUNCTION };

        ScriptFunction(const std::string& fname, const NativeFunction& nfunc, bool is_class = false);
        ScriptFunction(const std::string& fname, const std::vector<std::string>& args, 
            const std::vector<std::uint8_t>& bc, bool is_c = false, bool is_g = false);

        std::shared_ptr<ScriptFunction> Copy(const std::shared_ptr<Environment>& env) const;
        void Bind(const ScriptAny& this_obj);
        std::shared_ptr<ScriptFunction> BindCopy(const ScriptAny& this_obj) const;
        size_t GetHash() const override;

        static bool IsInstanceOf(const std::shared_ptr<ScriptObject>& obj, 
            const std::shared_ptr<ScriptFunction>& clazz);

        bool operator<(const ScriptFunction& func) const;
        bool operator==(const ScriptFunction& func) const;
        
        Type type() const { return type_; }
        const std::string& function_name() const { return function_name_; }
        const std::vector<std::string>& arg_names() const { return arg_names_; }
        const std::vector<std::uint8_t>&  compiled() const { return compiled_; }
        ScriptAny bound_this() const { return bound_this_; }
        auto closure() const { return closure_; }
        bool is_class() const { return is_class_; }
        bool is_generator() const { return is_generator_; }
        auto native_function() const { return native_function_; }
    private:
        void InitializePrototypeProperty();

        Type type_;
        std::string function_name_;
        std::vector<std::string> arg_names_;
        ScriptAny bound_this_;
        std::shared_ptr<Environment> closure_;
        bool is_class_;
        bool is_generator_;
        // std::shared_ptr<ConstTable> const_table_;
        NativeFunction native_function_;
        std::vector<std::uint8_t> compiled_;
    };

    std::ostream& operator<<(std::ostream& os, const ScriptFunction& func);
}