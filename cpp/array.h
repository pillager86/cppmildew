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

#include <iostream>
#include <memory>
#include <type_traits>

namespace cpp
{

inline size_t NextPowerOf2(const size_t n)
{
    size_t power = 1;
    while(power < n && power != 0)
        power <<= 1;
    return power;
}

/**
 * This template class implements a sliceable array that shares memory to the allocated array across slices
 */
template <typename T>
class Array
{
public:
    static constexpr bool is_utf8string = std::is_same_v<char, std::remove_cv_t<T>>;

    Array()
    {
        ptr_ = std::shared_ptr<T>(nullptr);
    }

    Array(std::initializer_list<T> list)
    {
        capacity_ = NextPowerOf2(list.size());
        Realloc(capacity_);
        for(const auto& item : list)
            Push(item);
    }

    Array(const std::string& str)
    {
        static_assert(std::is_same_v<char, std::remove_cv_t<T>>);
        capacity_ = NextPowerOf2(str.size());
        Realloc(capacity_);
        for(auto c : str)
            Push(c);
    }

    ~Array()
    {
        ptr_ = nullptr;
    }

    Array<T> Slice(size_t begin, size_t end=-1)
    {
        Array<T> newArray;
        newArray.ptr_ = ptr_;
        newArray.start_ = start_ + begin;
        newArray.length_ = end == static_cast<size_t>(-1) ? length_ : end - newArray.start_;
        newArray.is_slice_ = true;
        newArray.capacity_ = newArray.length_;
        return newArray;
    }

    const T& At(size_t index) const 
    {
        return *(ptr_.get() + start_ + index);
    }

    size_t Length() const
    {
        return length_;
    }

    void Push(const T& item)
    {
        if(is_slice_ || capacity_ <= length_)
        {
            Realloc(1);
            ptr_.get()[length_] = item;
        }
        else 
        {
            ptr_.get()[length_] = item;
        }
        ++length_;
    }

    T& operator[](size_t index)
    {
        return *(ptr_.get() + start_ + index);
    }

private:

    void Realloc(const size_t numToAdd)
    {
        auto newCapacity = capacity_ ? NextPowerOf2(length_ - start_ + numToAdd) : 8;
        auto newPtr = new T[newCapacity];
        for(size_t i = start_, j = 0; i < length_ - start_; ++i, ++j)
        {
            newPtr[j] = ptr_.get()[i];
        }
        start_ = 0;
        is_slice_ = false;
        capacity_ = newCapacity;
        ptr_ = std::shared_ptr<T>(newPtr, [](const T* p) { delete[] p; });
    }

    size_t capacity_ = 0;
    size_t start_ = 0;
    size_t length_ = 0;
    bool is_slice_ = false;
    std::shared_ptr<T> ptr_;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const cpp::Array<T>& array)
{
    if constexpr(Array<T>::is_utf8string)
    {
        for(size_t i = 0; i < array.Length(); ++i)
            os << array.At(i);
    }
    else 
    {
        os << "{";
        for(size_t i = 0; i < array.Length(); ++i)
        {
            os << array.At(i);
            if(i < array.Length() - 1)
                os << ", ";
        }
        os << "}";
    }
    return os;
}

} // namespace cpp