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

#include <functional>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <iostream>

namespace cppd
{

class RTTINode
{
    RTTINode(const std::type_info& base_t)
    : type_(base_t)
    { }

    template <class Derived, class Base>
    void SetDiff()
    {
        auto derived = reinterpret_cast<Derived*>(1);
        auto base = static_cast<Base*>(derived);
        diff_ = reinterpret_cast<intptr_t>(base) - reinterpret_cast<intptr_t>(derived);
        std::cout << "Diff " << typeid(Base).name() << " - " << typeid(Derived).name() << " = " << diff_ << std::endl;
    }

    const std::type_info& type_;
    ptrdiff_t diff_ = 0;

    friend class Object;
};

class Object
{
public:
    Object(void* p, const std::type_info& t, std::function<void()> destructor)
    : ptr_(p), type_(t), destructor_(destructor)
    {}

    template<class C>
    Object(C* ptr)
    : ptr_(ptr), type_(typeid(C)), destructor_([ptr] { delete ptr; })
    {}

    Object(const Object&) = delete;
    Object& operator=(const Object &) = delete;

    ~Object()
    {
        destructor_();
    }

    template<class C, class...Parents>
    static void RegisterClass()
    {
        if constexpr(sizeof...(Parents) == 0)
        {
            RegisterClassNoParent<C>();
        }
        else 
        {
            (RegisterClassParent<C, Parents>(), ...);
        }
    }

    template<class C, class Parent>
    static void RegisterClassParent()
    {
        std::cout << "Registering class " << typeid(C).name() << std::endl;
        using BaseClass = std::remove_cv_t<C>;
        using ParentClass = std::remove_cv_t<Parent>;
        if(hierarchy_.count(&typeid(C)) == 0)
        {
            auto new_vector = std::vector<RTTINode*>();
            auto node = new RTTINode(typeid(ParentClass));
            node->SetDiff<BaseClass, ParentClass>();
            new_vector.emplace_back(node);
            hierarchy_[&typeid(BaseClass)] = new_vector;
        }
        else // have to add to existing vector 
        {
            // auto parent_list = hierarchy_[&typeid(C)];
            auto node = new RTTINode(typeid(ParentClass));
            node->SetDiff<BaseClass, ParentClass>();
            hierarchy_[&typeid(BaseClass)].emplace_back(node);
        } 
    }

    template<class C> // no parent
    static void RegisterClassNoParent()
    {
        using BaseClass = std::remove_cv_t<C>;
        std::cout << "Registering parentless class " << typeid(BaseClass).name() << std::endl;
        if(hierarchy_.count(&typeid(BaseClass)) > 0)
            return; // nothing to do
        hierarchy_[&typeid(BaseClass)] = std::vector<RTTINode*>();
    }

    template<class Base>
    Base* Cast()
    {
        // using RegisteredClass = std::remove_cv_t<Base>; // TODO fix: this is busted
        if(typeid(Base).hash_code() == type_.hash_code())
            return reinterpret_cast<Base*>(ptr_); // nothing else to do already same type
        // is this a valid upcast?
        auto result = SearchTree(type_, typeid(Base));
        if(result == -1)
            return nullptr;
        return reinterpret_cast<Base*>(reinterpret_cast<char*>(ptr_) + result);
    }

private:
    static int SearchTree(const std::type_info& derived_t, const std::type_info& base_t, int accumulator=0);

    void* ptr_;
    const std::type_info& type_;
    std::function<void()> destructor_;

    static std::unordered_map<const std::type_info*, std::vector<RTTINode*>> hierarchy_;
};

template<class C, typename... Args>
Object* MakeObject(Args... args)
{
    C* c = new C(std::forward<Args>(args)...);
    return new Object(c, typeid(C), [c] { delete c; });
}

} // namespace cppd