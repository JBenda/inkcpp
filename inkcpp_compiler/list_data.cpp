#include "list_data.h"

#include <algorithm>

namespace ink::compiler::internal
{
	void list_data::new_list(const std::string& list_name)
	{
		_lists.insert({list_name, static_cast<int>(_list_end.size())});
		_list_name.emplace_back(list_name);
		int current_back = _list_end.empty() ? 0 : _list_end.back();
		_current_list_start = current_back;
		_list_end.push_back(current_back);
	}

	void list_data::new_flag(const std::string& flag_name, size_t value)
	{
		while(_flag_names.size() < _current_list_start + value) {
			_flag_names.push_back("");
		}
		if(_current_list_start + value > _list_end.back()) {
			_list_end.back() = _current_list_start + value;
		}
		_flag_names[_current_list_start + value - 1] = flag_name;
	}

	std::vector<list_data::named_list_flag> list_data::get_flags() const {
		std::vector<named_list_flag> result{};
		size_t begin = 0;
		for(size_t i = 0; i < _list_end.size(); ++i) {
			for(size_t j = begin; j < _list_end[i]; ++j) {
				if (_flag_names[j] != "") {
					result.push_back({
							_flag_names[j],
							{	static_cast<lid_t>(i),
								static_cast<flag_t>(j - begin)}
							});
				}
			}
			begin = _list_end[i];
		}
		return result;
	}
}
