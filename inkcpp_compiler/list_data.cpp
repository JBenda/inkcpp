/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "list_data.h"

#include <algorithm>
#include <limits>

namespace ink::compiler::internal
{
void list_data::new_list(const std::string& list_name)
{
	_lists.insert({list_name, static_cast<int>(_list_end.size())});
	_list_name.emplace_back(list_name);
	int current_back = _list_end.empty() ? 0 : _list_end.back();
	_list_end.push_back(current_back);
}

void list_data::new_flag(const std::string& flag_name, int value)
{
	inkAssert(
	    value <= std::numeric_limits<decltype(list_flag::flag)>::max()
	        && value >= std::numeric_limits<decltype(list_flag::flag)>::min(),
	    "Value outside of current supported scope"
	);
	_list_end.back() += 1;
	_flags.emplace_back(
	    &flag_name,
	    list_flag{
	        static_cast<decltype(list_flag::list_id)>(_list_name.size() - 1),
	        static_cast<decltype(list_flag::flag)>(value)
	    }
	);
}

void list_data::sort()
{
	size_t begin = 0;
	for (size_t i = 0; i < _list_end.size(); ++i) {
		std::sort(_flags.begin() + begin, _flags.begin() + _list_end[i]);
		begin = _list_end[i];
	}
}

} // namespace ink::compiler::internal
