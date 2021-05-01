#include "list_data.h"

namespace ink::compiler::internal
{
	void list_data::new_list(const std::string& list_name)
	{
		if(_current_list_start) {
			_list_len.push_back(_flag_names.size() - _current_list_start);
		}
		_current_list_start = _flag_names.size();
		_lists[list_name] = _lists.size();
	}

	void list_data::new_flag(const std::string& flag_name, int value)
	{
		while(_flag_names.size() > _current_list_start + value) {
			_flag_names.push_back("");
		}
		_flag_names[_current_list_start + value - 1] = flag_name;
	}
}
