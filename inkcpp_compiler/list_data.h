#pragma once

#include "system.h"

#include <vector>
#include <string>
#include <map>
#include <string_view>

namespace ink::compiler::internal
{
	class list_data {
		using flag_t = decltype(list_flag::flag);
		using lid_t = decltype(list_flag::list_id);

	public:

		// add new list and set it to current
		void new_list(const std::string& list_name);

		// add flag to current list
		void new_flag(const std::string& flag_name, size_t value);

		lid_t get_lid(const std::string_view& list_name) {
			auto itr = _lists.find(list_name);
			return static_cast<lid_t>(itr->second);
		}

		bool empty() const { return _lists.empty(); }
		struct named_list_flag {
			const std::string& name;
			list_flag flag;
		};
		std::vector<named_list_flag> get_flags() const;
		const std::vector<std::string_view>& get_list_names() const {
			return _list_name;
		}
	private:
		std::map<std::string, int,std::less<>> _lists;
		std::vector<size_t> _list_end;
		std::vector<std::string_view> _list_name;
		size_t _current_list_start = 0;
		std::vector<std::string> _flag_names;
	};
}
