/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "list_impl.h"
#include "include/list.h"
#include "list_table.h"

namespace ink::runtime::internal
{
bool list_impl::contains(const char* flag_name) const
{
	auto flag = _list_table->toFlag(flag_name);
	inkAssert(flag.has_value(), "No flag with name found! " FORMAT_STRING_STR "'", flag_name);
	return _list_table->has(list_table::list{_list}, *flag);
}

void list_impl::add(const char* flag_name)
{
	auto flag = _list_table->toFlag(flag_name);
	inkAssert(flag.has_value(), "No flag with name found to add! '" FORMAT_STRING_STR "'", flag_name);
	_list = _list_table->add(list_table::list{_list}, *flag).lid;
}

void list_impl::remove(const char* flag_name)
{
	auto flag = _list_table->toFlag(flag_name);
	inkAssert(
	    flag.has_value(), "No flag with name found to remove! '" FORMAT_STRING_STR "'", flag_name
	);
	_list = _list_table->sub(list_table::list{_list}, *flag).lid;
}

void list_impl::next(const char*& flag_name, const char*& list_name, int& i, bool one_list_only)
    const
{
	if (i == -1) {
		return;
	}

	list_flag flag{static_cast<int16_t>(i >> 16), static_cast<int16_t>(i & 0x7FFF)};
	if (flag_name != nullptr) {
		++flag.flag;
	}
	if (flag.list_id < 0 || static_cast<size_t>(flag.list_id) >= _list_table->_list_end.size()
	    || static_cast<size_t>(flag.flag)
	           >= (_list_table->_list_end[flag.list_id] - _list_table->listBegin(flag.list_id))) {
next_list:
		if (one_list_only) {
			i = -1;
			return;
		}
		flag.flag = 0;
		do {
			++flag.list_id;
			if (flag.list_id < 0 || static_cast<size_t>(flag.list_id) >= _list_table->_list_end.size()) {
				i = -1;
				return;
			}
		} while (! _list_table->hasList(_list_table->getPtr(_list), flag.list_id));
	}
	while (! _list_table->has(list_table::list{_list}, flag)) {
		++flag.flag;
		if (static_cast<size_t>(flag.flag)
		    >= _list_table->_list_end[flag.list_id] - _list_table->listBegin(flag.list_id)) {
			goto next_list;
		}
	}
	int fid   = _list_table->toFid(flag);
	flag_name = (fid >= 0 && static_cast<size_t>(fid) < _list_table->_flag_names.size())
	              ? _list_table->_flag_names[fid]
	              : nullptr;
	list_name
	    = (flag.list_id >= 0 && static_cast<size_t>(flag.list_id) < _list_table->_list_names.size())
	        ? _list_table->_list_names[flag.list_id]
	        : nullptr;

	i = (flag.list_id << 16) | (flag.flag & 0x7FFF);
}

list_interface::iterator list_impl::begin(const char* list_name) const
{
	list_flag lf = _list_table->get_list_id(list_name);
	if (lf.list_id < 0) {
		return end();
	}
	return ++new_iterator(nullptr, lf.list_id << 16, true);
}
} // namespace ink::runtime::internal
