#pragma once

#include "system.h"

#include <vector>
#include <string>
#include <map>
#include <string_view>

namespace ink::compiler::internal
{
	class list_data {
	public:

		// add new list and set it to current
		void new_list(const std::string& list_name);

		// add flag to current list
		void new_flag(const std::string& flag_name, int value);

		int get_lid(const std::string_view& list_name) {
			auto itr = _lists.find(list_name);
			return static_cast<decltype(list_flag::list_id)>(itr->second);
		}

	private:
		std::map<std::string, int,std::less<>> _lists;
		std::vector<std::string> _flag_names;
		std::vector<int> _list_len;
		int _current_list_start = 0;
	};
}
