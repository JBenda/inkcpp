/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "system.h"

#include <vector>
#include <string>
#include <map>
#include <string_view>

namespace ink::compiler::internal
{
class list_data
{
	using flag_t = decltype(list_flag::flag);
	using lid_t  = decltype(list_flag::list_id);

public:
	// add new list and set it to current
	void new_list(const std::string& list_name);

	// add flag to current list
	void new_flag(const std::string& flag_name, int value);

	// sort flags per list
	void sort();

	lid_t get_lid(const std::string_view& list_name)
	{
		auto itr = _lists.find(list_name);
		return static_cast<lid_t>(itr->second);
	}

	bool empty() const { return _lists.empty(); }

	struct named_list_flag {
		named_list_flag(const std::string* name, list_flag flag)
		    : name{name}
		    , flag{flag}
		{
		}

		const std::string* name;
		list_flag          flag;

		bool operator<(const named_list_flag& oth) const
		{
			inkAssert(
			    flag.list_id == oth.flag.list_id, "Compare flags from different lists is not supported"
			);
			return flag.flag < oth.flag.flag;
		}
	};

	const std::vector<named_list_flag>& get_flags() const { return _flags; }

	const std::vector<std::string_view>& get_list_names() const { return _list_name; }

private:
	std::map<std::string, int, std::less<>> _lists;
	std::vector<std::string_view>           _list_name;
	std::vector<int>                        _list_end;
	std::vector<named_list_flag>            _flags;
};
} // namespace ink::compiler::internal
