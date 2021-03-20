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

namespace cpp 
{
std::unordered_map<const std::type_info*, std::vector<RTTINode*>> Object::hierarchy_;

int Object::SearchTree(const std::type_info& derived_t, const std::type_info& base_t, int accumulator)
{
    // derived must have an entry
    if(Object::hierarchy_.count(&derived_t) == 0)
        return -1;
    auto parent_list = Object::hierarchy_[&derived_t];
    std::cout << "Found parent list of " << derived_t.name() << std::endl;
    std::cout << " Which has " << parent_list.size() << " elements" << std::endl;
    for(const auto* parent_entry : parent_list)
    {
        if(parent_entry->type_.hash_code() == base_t.hash_code())
        {
            std::cout << "Found immediate parent" << std::endl;
            return accumulator + parent_entry->diff_;
        }
        else
        {
            std::cout << "Recursively searching " << parent_entry->type_.name() << std::endl;
            auto result = Object::SearchTree(parent_entry->type_, base_t, parent_entry->diff_ + accumulator);
            if(result != -1)
                return result;
        }
    }
    return -1;
}

} // namespace cpp