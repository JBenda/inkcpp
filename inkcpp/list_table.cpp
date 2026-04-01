/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "list_table.h"
#include "config.h"
#include "hungarian_solver.h"
#include "system.h"
#include "traits.h"
#include "header.h"
#include "random.h"
#include "string_utils.h"
#include "list_impl.h"
#include <limits>

#ifdef INK_ENABLE_STL
#	include <ostream>
#endif

namespace ink::runtime::internal
{

void list_table::copy_lists(const data_t* src, data_t* dst)
{
	int len  = numLists() / bits_per_data;
	int rest = numLists() % bits_per_data;
	for (int i = 0; i < len; ++i) {
		dst[i] = src[i];
	}
	if (rest) {
		dst[len] |= src[len] & (~static_cast<data_t>(0) << (bits_per_data - rest));
	}
}

list_table::list_table(const char* data, const ink::internal::header& header)
    : _valid{false}
{
	if (data == nullptr) {
		return;
	}
	list_flag   flag;
	const char* ptr   = data;
	int         start = 0;
	while ((flag = header.read_list_flag(ptr)) != null_flag) {
		// start of new list
		if (static_cast<int16_t>(_list_end.size()) == flag.list_id) {
			start              = _list_end.size() == 0 ? 0 : _list_end.back();
			_list_end.push()   = start;
			_list_names.push() = ptr;
			while (*ptr) {
				++ptr;
			}
			++ptr; // skip string
		}
		_flag_names.push()  = ptr;
		_flag_values.push() = flag.flag;
		++_list_end.back();
		while (*ptr) {
			++ptr;
		}
		++ptr; // skip string
	}
	_entrySize = segmentsFromBits(_list_end.size() + _flag_names.size(), sizeof(data_t));
	_valid     = true;
}

list_table::list list_table::create()
{
	for (size_t i = 0; i < _entry_state.size(); ++i) {
		if (_entry_state[i] == state::empty) {
			_entry_state[i] = state::used;
			return list(i);
		}
	}

	list new_entry(_entry_state.size());
	// TODO: initialized unused?
	_entry_state.push() = state::used;
	for (int i = 0; i < _entrySize; ++i) {
		_data.push() = 0;
	}
	return new_entry;
}

list_table::list list_table::create_at(size_t idx)
{
	if (idx < _entry_state.size()) {
		if (_entry_state[idx] == state::empty) {
			_entry_state[idx] = state::used;
			return list(idx);
		}
		return list(-1);
	}
	while (_entry_state.size() <= idx) {
		_entry_state.push() = state::empty;
		for (int i = 0; i < _entrySize; ++i) {
			_data.push() = 0;
		}
	}
	return list(idx);
}

void list_table::clear_usage()
{
	for (state& s : _entry_state) {
		if (s == state::used) {
			s = state::unused;
		}
	}
}

void list_table::mark_used(list l)
{
	if (_entry_state[l.lid] == state::unused) {
		_entry_state[l.lid] = state::used;
	}
}

void list_table::gc()
{
	for (size_t i = 0; i < _entry_state.size(); ++i) {
		if (_entry_state[i] == state::unused) {
			_entry_state[i] = state::empty;
			data_t* entry   = getPtr(i);
			for (int j = 0; j != _entrySize; ++j) {
				entry[j] = 0;
			}
		}
	}
	_list_handouts.clear();
}

size_t list_table::toFid(list_flag e) const { return listBegin(e.list_id) + e.flag; }

size_t list_table::stringLen(const list_flag& e) const { return c_str_len(toString(e)); }

const char* list_table::toString(const list_flag& e) const
{
	if (e.list_id < 0 || e.flag < 0) {
		return "";
	}
	const char* res = _flag_names[toFid(e)];
	return res == nullptr ? "" : res;
}

size_t list_table::stringLen(const list& l) const
{
	size_t        len   = 0;
	const data_t* entry = getPtr(l.lid);
	bool          first = true;
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(entry, i)) {
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				if (hasFlag(entry, j) && _flag_names[j]) {
					if (! first) {
						len += 2; // ', '
					} else {
						first = false;
					}
					len += c_str_len(_flag_names[j]);
				}
			}
		}
	}
	return len;
}

/// @todo check ouput order for explicit valued lists
/// @sa list_table::write()
char* list_table::toString(char* out, const list& l) const
{
	char* itr = out;

	const data_t* entry      = getPtr(l.lid);
	int           last_value = 0;
	int           last_list  = -1;
	bool          first      = true;
	int           min_value  = 0;
	int           min_id     = -1;
	int           min_list   = -1;

	while (1) {
		bool change = false;
		for (size_t i = 0; i < numLists(); ++i) {
			if (hasList(entry, i)) {
				for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
					if (! hasFlag(entry, j)) {
						continue;
					}
					int value = _flag_values[j];
					// the cast is ok, since if we are in the
					// first round, `first` is true and we do not evaluate
					// second round, `last_list` is >= 0
					if (first || value > last_value
					    || (value == last_value && i > static_cast<size_t>(last_list))) {
						if (min_id == -1 || value < min_value) {
							change    = true;
							min_list  = i;
							min_value = value;
							min_id    = j;
						}
						break;
					}
				}
			}
		}
		if (! change) {
			break;
		}
		if (! first) {
			*itr++ = ',';
			*itr++ = ' ';
		}
		first = false;
		for (const char* c = _flag_names[min_id]; *c; ++c) {
			*itr++ = *c;
		}
		last_value = min_value;
		last_list  = min_list;
		min_id     = -1;
	}
	return itr;
}

list_table::list list_table::range(list_table::list l, int min, int max)
{
	list    res          = create();
	data_t* in           = getPtr(l.lid);
	data_t* out          = getPtr(res.lid);
	bool    has_any_list = false;
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(in, i)) {
			bool has_flag = false;
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				int value = _flag_values[j];
				if (value < min) {
					continue;
				}
				if (value > max) {
					break;
				}
				if (hasFlag(in, j)) {
					setFlag(out, j);
					has_flag = true;
				}
			}
			if (has_flag) {
				has_any_list = true;
				setList(out, i);
			}
		}
	}
	if (has_any_list) {
		return res;
	}
	copy_lists(in, out);
	return res;
}

list_table::list list_table::add(list_flag lh, list_flag rh)
{
	list    res = create();
	data_t* o   = getPtr(res.lid);
	setList(o, lh.list_id);
	setFlag(o, toFid(lh));
	setList(o, rh.list_id);
	setFlag(o, toFid(rh));
	return res;
}

list_table::list list_table::add(list lh, list rh)
{
	list    res = create();
	data_t* l   = getPtr(lh.lid);
	data_t* r   = getPtr(rh.lid);
	data_t* o   = getPtr(res.lid);
	for (int i = 0; i < _entrySize; ++i) {
		o[i] = l[i] | r[i];
	}
	return res;
}

list_table::list list_table::create_permament()
{
	list res              = create();
	_entry_state[res.lid] = state::permanent;
	return res;
}

list_table::list list_table::create_permament_at(size_t idx)
{
	list res = create_at(idx);
	if (res.lid >= 0) {
		_entry_state[res.lid] = state::permanent;
	}
	return res;
}

list_table::list& list_table::add_inplace(list& lh, list_flag rh)
{
	if (rh.list_id < 0)
		return lh; // empty or null flag (skip)
	data_t* l = getPtr(lh.lid);
	setList(l, rh.list_id);
	if (rh.flag >= 0) { // origin entry
		setFlag(l, toFid(rh));
	}
	return lh;
}

list_table::list list_table::add(list lh, list_flag rh)
{
	list    res = create();
	data_t* l   = getPtr(lh.lid);
	data_t* o   = getPtr(res.lid);
	for (int i = 0; i < _entrySize; ++i) {
		o[i] = l[i];
	}
	setList(o, rh.list_id);
	setFlag(o, toFid(rh));
	return res;
}

list_table::list list_table::sub(list lh, list rh)
{
	list    res         = create();
	data_t* l           = getPtr(lh.lid);
	data_t* r           = getPtr(rh.lid);
	data_t* o           = getPtr(res.lid);
	bool    active_flag = false;
	for (int i = 0; i < _entrySize; ++i) {
		o[i] = (l[i] & r[i]) ^ l[i];
	}

	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(r, i)) {
			if (hasList(l, i)) {
				for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
					if (hasFlag(o, j)) {
						setList(o, i);
						active_flag = true;
						break;
					}
				}
			}
		}
	}
	if (active_flag) {
		return res;
	}
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(o, i)) {
			return res;
		}
	}
	copy_lists(l, o);
	return res;
}

list_table::list list_table::sub(list lh, list_flag rh)
{
	list    res = create();
	data_t* l   = getPtr(lh.lid);
	data_t* o   = getPtr(res.lid);
	for (int i = 0; i < _entrySize; ++i) {
		o[i] = l[i];
	}
	setFlag(o, toFid(rh), false);
	for (size_t i = listBegin(rh.list_id); i < _list_end[rh.list_id]; ++i) {
		if (hasFlag(o, i)) {
			return res;
		}
	}
	setList(l, rh.list_id, false);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(o, i)) {
			return res;
		}
	}
	copy_lists(l, o);
	return res;
}

list_flag list_table::sub(list_flag lh, list rh)
{
	data_t* r = getPtr(rh.lid);
	if (hasList(r, lh.list_id) && hasFlag(r, toFid(lh))) {
		return list_flag{lh.list_id, -1};
	}
	return lh;
}

/// @todo early exit if value + n is outside of range
list_table::list list_table::add(list arg, int n)
{
	// TODO: handle i == 0 (for performance only)
	if (n < 0) {
		return sub(arg, -n);
	}
	list    res         = create();
	data_t* l           = getPtr(arg.lid);
	data_t* o           = getPtr(res.lid);
	bool    active_flag = false;
	;
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(l, i)) {
			bool has_flag = false;
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				if (hasFlag(l, j)) {
					int value = _flag_values[j] + n;
					for (size_t k = j + 1; k < _list_end[i]; ++k) {
						if (value == _flag_values[k]) {
							setFlag(o, k);
							has_flag = true;
							break;
						}
					}
				}
			}
			if (has_flag) {
				active_flag = true;
				setList(o, i);
			}
		}
	}
	if (! active_flag) {
		copy_lists(l, o);
	}
	return res;
}

list_flag list_table::add(list_flag arg, int n)
{
	if (arg == null_flag || arg == empty_flag || arg.flag == -1) {
		return arg;
	}
	int value = _flag_values[arg.flag] + n;
	for (size_t i = listBegin(arg.list_id); i < _list_end[arg.list_id]; ++i) {
		if (_flag_values[i] == value) {
			arg.flag = static_cast<int16_t>(i);
			return arg;
		}
	}
	arg.flag = -1;
	return arg;
}

/// @todo early exit if value - n is outside of range
list_table::list list_table::sub(list arg, int n)
{
	// TODO: handle i == 0 (for perofrgmance only)
	if (n < 0) {
		return add(arg, -n);
	}
	list    res         = create();
	data_t* l           = getPtr(arg.lid);
	data_t* o           = getPtr(res.lid);
	bool    active_flag = false;
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(l, i)) {
			bool has_flag = false;
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				if (hasFlag(l, j)) {
					int value = _flag_values[j] - n;
					for (size_t k = j - 1; k != ~0U && k >= listBegin(i) && k != ~0U; --k) {
						if (_flag_values[k] == value) {
							setFlag(o, k);
							has_flag = true;
							break;
						}
					}
				}
			}
			if (has_flag) {
				active_flag = true;
				setList(o, i);
			}
		}
	}
	if (! active_flag) {
		copy_lists(l, o);
	}
	return res;
}

list_flag list_table::sub(list_flag arg, int i) { return add(arg, -i); }

int32_t list_table::count(list_flag lf) const
{
	if (lf == empty_flag || lf == null_flag || lf.flag == -1) {
		return 0;
	}
	if (_flag_names[toFid(lf)] == nullptr) {
		return 0;
	}
	return 1;
}

int32_t list_table::count(list l) const
{
	int           count = 0;
	const data_t* data  = getPtr(l.lid);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(data, i)) {
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				if (_flag_names[j] != nullptr && hasFlag(data, j)) {
					++count;
				}
			}
		}
	}
	return count;
}

list_flag list_table::min(list l) const
{
	list_flag     res{-1, -1};
	const data_t* data = getPtr(l.lid);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(data, i)) {
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				if (hasFlag(data, j)) {
					int value = _flag_values[j];
					if (res.flag < 0 || value < res.flag) {
						res.flag    = static_cast<int16_t>(value);
						res.list_id = static_cast<int16_t>(i);
					}
					break;
				}
			}
		}
	}
	return res;
}

list_flag list_table::max(list l) const
{
	list_flag     res{-1, -1};
	const data_t* data = getPtr(l.lid);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(data, i)) {
			for (size_t j = _list_end[i] - 1; j != ~0U && j >= listBegin(i); --j) {
				if (hasFlag(data, j)) {
					int value = _flag_values[j];
					if (value > res.flag) {
						res.flag    = static_cast<int16_t>(value);
						res.list_id = static_cast<int16_t>(i);
					}
					break;
				}
			}
		}
	}
	return res;
}

bool list_table::equal(list lh, list rh) const
{
	const data_t* l = getPtr(lh.lid);
	const data_t* r = getPtr(rh.lid);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(l, i) != hasList(r, i)) {
			return false;
		}
		if (hasList(l, i)) {
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				if (hasFlag(l, j) != hasFlag(r, j)) {
					return false;
				}
			}
		}
	}
	return true;
}

bool list_table::equal(list lh, list_flag rh) const
{
	const data_t* l = getPtr(lh.lid);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(l, i) != (rh.list_id == static_cast<int16_t>(i))) {
			return false;
		}
	}
	for (size_t i = listBegin(rh.list_id); i < _list_end[rh.list_id]; ++i) {
		if (hasFlag(l, i) != (rh.flag == static_cast<int16_t>(i - listBegin(rh.list_id)))) {
			return false;
		}
	}
	return true;
}

list_table::list list_table::all(list arg)
{
	list    res = create();
	data_t* l   = getPtr(arg.lid);
	data_t* o   = getPtr(res.lid);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(l, i)) {
			setList(o, i);
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				setFlag(o, j);
			}
		}
	}
	return res;
}

list_table::list list_table::all(list_flag arg)
{
	list res = create();
	if (arg != null_flag) {
		data_t* o = getPtr(res.lid);
		setList(o, arg.list_id);
		for (size_t i = listBegin(arg.list_id); i < _list_end[arg.list_id]; ++i) {
			setFlag(o, i);
		}
	}
	return res;
}

// ATTENTION: can produce an list without setted flag list (same behavior than inklecate)
list_table::list list_table::invert(list arg)
{
	list    res = create();
	data_t* l   = getPtr(arg.lid);
	data_t* o   = getPtr(res.lid);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(l, i)) {
			bool hasList = false;
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				bool have = hasFlag(l, j);
				if (! have) {
					hasList = true;
					setFlag(o, j);
				}
			}
			if (hasList) {
				setList(o, i);
			}
		}
	}
	return res;
}

list_table::list list_table::invert(list_flag arg)
{
	list res = create();
	if (arg != null_flag) {
		data_t* o = getPtr(res.lid);
		for (size_t i = listBegin(arg.list_id); i < _list_end[arg.list_id]; ++i) {
			setFlag(o, i, arg.flag != static_cast<int16_t>(i - listBegin(arg.list_id)));
		}
	}
	return res;
}

list_table::list list_table::intersect(list lh, list rh)
{
	list    res = create();
	data_t* l   = getPtr(lh.lid);
	data_t* r   = getPtr(rh.lid);
	data_t* o   = getPtr(res.lid);
	for (int i = 0; i < _entrySize; ++i) {
		o[i] = l[i] & r[i];
	}
	return res;
}

list_flag list_table::intersect(list lh, list_flag rh)
{
	const data_t* l = getPtr(lh.lid);
	if (hasList(l, rh.list_id) && hasFlag(l, toFid(rh))) {
		return rh;
	}
	return null_flag;
}

bool list_table::has(list lh, list_flag rh) const
{
	const data_t* l = getPtr(lh.lid);
	return hasList(l, rh.list_id) && hasFlag(l, toFid(rh));
}

list_flag list_table::lrnd(list lh, prng& rng) const
{
	const data_t* l = getPtr(lh.lid);
	int           n = count(lh);
	n               = rng.rand(n);
	int count       = 0;
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(l, i)) {
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				if (hasFlag(l, j)) {
					if (count++ == n) {
						return list_flag{
						    static_cast<decltype(list_flag::list_id)>(i),
						    static_cast<decltype(list_flag::flag)>(j - listBegin(i))
						};
					}
				}
			}
		}
	}
	return null_flag;
}

bool list_table::has(list lh, list rh) const
{
	const data_t* r = getPtr(rh.lid);
	const data_t* l = getPtr(lh.lid);
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(r, i)) {
			if (! hasList(l, i)) {
				return false;
			}
			for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
				if (hasFlag(r, j) && ! hasFlag(l, j)) {
					return false;
				}
			}
		}
	}
	return true;
}

optional<list_flag> list_table::toFlag(const char* flag_name) const
{

	const char* periode = str_find(flag_name, '.');
	if (periode) {
		list_flag list = get_list_id(flag_name); // since flag_name is `list_name.flag_name`
		flag_name      = periode + 1;
		int list_begin = list.list_id == 0 ? 0 : _list_end[list.list_id - 1];
		for (size_t i = list_begin; i != _list_end[list.list_id]; ++i) {
			if (str_equal(flag_name, _flag_names[i])) {
				return {
				    list_flag{list.list_id, static_cast<int16_t>(i - list_begin)}
				};
			}
		}
	} else {
		for (auto flag_itr = _flag_names.begin(); flag_itr != _flag_names.end(); ++flag_itr) {
			if (str_equal(*flag_itr, flag_name)) {
				size_t fid   = static_cast<size_t>(flag_itr - _flag_names.begin());
				size_t lid   = 0;
				int    begin = 0;
				for (auto* list_itr = _list_end.begin(); list_itr != _list_end.end(); ++list_itr) {
					if (*list_itr > fid) {
						lid = static_cast<size_t>(list_itr - _list_end.begin());
						break;
					}
					begin = *list_itr;
				}
				return {
				    list_flag{static_cast<int16_t>(lid), static_cast<int16_t>(fid - begin)}
				};
			}
		}
	}
	return nullopt;
}

list_flag list_table::get_list_id(const char* list_name) const
{
	using int_t        = decltype(list_flag::list_id);
	const char* period = str_find(list_name, '.');
	size_t      len    = period ? static_cast<size_t>(period - list_name) : c_str_len(list_name);
	for (int_t i = 0; i < static_cast<int_t>(_list_names.size()); ++i) {
		if (str_equal_len(list_name, _list_names[i], len)) {
			return list_flag{i, -1};
		}
	}
	inkAssert(false, "No list with name found!");
	return null_flag;
}

list_table::list list_table::redefine(list lh, list rh)
{
	data_t* l   = getPtr(lh.lid);
	data_t* r   = getPtr(rh.lid);
	list    res = create();
	data_t* o   = getPtr(res.lid);

	// if the new list has no origin: give it the origin of the old value
	bool has_origin = false;
	for (size_t i = 0; i < numLists(); ++i) {
		if (hasList(r, static_cast<int>(i))) {
			has_origin = true;
			break;
		}
	}
	if (! has_origin) {
		copy_lists(l, r);
	}

	for (int i = 0; i < _entrySize; ++i) {
		o[i] = r[i];
	}
	return res;
}

list_interface* list_table::handout_list(list l)
{
	static_assert(sizeof(list_interface) == sizeof(list_impl));
	auto& res = _list_handouts.push();
	new (&res) list_impl(*this, l.lid);
	return &res;
}

#ifdef INK_ENABLE_STL
/// @sa list_table::toString(char*,const list&)
std::ostream& list_table::write(std::ostream& os, list l) const
{
	const data_t* entry      = getPtr(l.lid);
	int           last_value = 0;
	int           last_list  = -1;
	bool          first      = true;
	int           min_value  = 0;
	int           min_id     = -1;
	int           min_list   = -1;

	while (1) {
		bool change = false;
		for (size_t i = 0; i < numLists(); ++i) {
			if (hasList(entry, static_cast<int>(i))) {
				for (size_t j = listBegin(i); j < _list_end[i]; ++j) {
					if (! hasFlag(entry, static_cast<int>(j))) {
						continue;
					}
					int value = _flag_values[j];
					// the cast is ok, since if we are in the
					// first round, `first` is true and we do not evaluate
					// second round, `last_list` is >= 0
					if (first || value > last_value
					    || (value == last_value && i > static_cast<size_t>(last_list))) {
						if (min_id == -1 || value < min_value) {
							min_value = value;
							min_id    = j;
							min_list  = i;
							change    = true;
						}
						break;
					}
				}
			}
		}
		if (! change) {
			break;
		}
		if (! first) {
			os << ", ";
		}
		first = false;
		os << _flag_names[min_id];
		last_value = min_value;
		last_list  = min_list;
		min_id     = -1;
	}
	return os;
}
#endif

size_t list_table::snap(unsigned char* data, const snapper& snapper) const
{
	unsigned char* ptr = data;
	ptr += _data.snap(data ? ptr : nullptr, snapper);
	ptr += _entry_state.snap(data ? ptr : nullptr, snapper);
	return static_cast<size_t>(ptr - data);
}

const unsigned char* list_table::snap_load(const unsigned char* ptr, const loader& loader)
{
	ptr = _data.snap_load(ptr, loader);
	ptr = _entry_state.snap_load(ptr, loader);
	return ptr;
}

config::statistics::list_table list_table::statistics() const
{
	return {
	    _list_handouts.statistics(),
	    _list_end.statistics(),
	    _flag_names.statistics(),
	    _entry_state.statistics(),
	};
}

/** Distance of two lists based on their contained values.
 * https://en.wikipedia.org/wiki/Jaccard_index
 * @param lh,rh flag indexes contained in the lists
 * @param matches mapping from lh -> rh, -1 for dropped
 */
float d_contains(const int lh[2], const int rh[2], const int* matches)
{
	int n_union        = (lh[1] - lh[0]) + (rh[1] - rh[0]);
	int n_intersection = 0;
	for (int i = lh[0]; i < lh[1]; ++i) {
		if (matches[i] >= rh[0] && matches[i] < rh[1]) {
			n_intersection += 1;
		}
	}
	n_union -= n_intersection;
	return static_cast<float>(n_intersection) / n_union;
}

/** Distance function for string labels.
 * @param lh,rh null terminated ASCII strings to compare
 * @return 0 if identical
 */
float d_label(const char* lh, const char* rh) { return 1.f - jaro_winkler_simularity(lh, rh); }

/** Distance function for two values.
 * @param lh,rh numeric values to compare
 * @param lh_range,rh_range min/max value of the number
 * @returns 0 if identical
 */
float d_value(int lh, int rh, int lh_range[2], int rh_range[2])
{
	if (lh == rh) {
		return 0;
	}
	float res = (static_cast<float>(lh) - lh_range[0]) / (lh_range[1] - lh_range[0]);
	res -= (static_cast<float>(rh) - rh_range[0]) / (rh_range[1] - rh_range[0]);
	if (res < 0) {
		res = -res;
	}
	return res;
}

struct MatchList {
	const size_t*      list_ends;
	const char* const* names;
	size_t             length;
};

struct MatchListValues {
	const char* const* names;
	const int*         values;
	size_t             length;
};

void get_range(const MatchListValues& values, int range[2])
{
	range[0] = std::numeric_limits<int>::max();
	range[1] = std::numeric_limits<int>::min();
	for (size_t i = 0; i < values.length; ++i) {
		if (values.values[i] < range[0]) {
			range[0] = values.values[i];
		}
		if (values.values[i] > range[1]) {
			range[1] = values.values[i];
		}
	}
}

float* cost_matrix(
    const MatchList& lh, const MatchList& rh, const int* value_matches, float drop_penalty
)
{
	size_t n_lists = lh.length > rh.length ? lh.length : rh.length;
	float* matrix  = new float[n_lists * n_lists];
	for (size_t i = 0; i < lh.length; ++i) {
		for (size_t j = 0; j < rh.length; ++j) {
			float dl         = d_label(lh.names[i], rh.names[j]);
			int   lh_range[] = {lh.list_ends[i], lh.list_ends[i + 1]};
			int   rh_range[] = {rh.list_ends[j], rh.list_ends[j + 1]};
			float dv         = d_contains(lh_range, rh_range, value_matches);
		}
	}
	return matrix;
}

float* cost_matrix(const MatchListValues& lh, const MatchListValues& rh, float drop_penalty)
{
	size_t n_flags = lh.length > rh.length ? lh.length : rh.length;
	float* matrix  = new float[n_flags * n_flags];
	int    lh_range[2], rh_range[2];
	get_range(lh, lh_range);
	get_range(rh, rh_range);

	for (size_t i = 0; i < lh.length; ++i) {
		for (size_t j = 0; j < rh.length; ++j) {
			float dl                = d_label(lh.names[i], rh.names[j]);
			float dv                = d_value(lh.values[i], rh.values[j], lh_range, rh_range);
			matrix[i * n_flags + j] = dl * 0.8 + dv * 0.2;
		}
		for (size_t j = rh.length; j < n_flags; ++j) {
			matrix[i * n_flags + j] = drop_penalty;
		}
	}
	for (size_t i = lh.length; i < n_flags; ++i) {
		for (size_t j = 0; j < rh.length; ++j) {
			matrix[i * n_flags + j] = drop_penalty;
		}
	}
	return matrix;
}

list_table::list_table(
    const char* data, const ink::internal::header&, const decltype(_data)& values,
    const decltype(_entry_state)& state
)
{
}

bool list_table::migrate(const char* old_list_metadata, const ink::internal::header& header)
{
	list_table old_ref_table(old_list_metadata, header, _data, _entry_state);
	_data.clear();
	_entry_state.clear();

	// find best mapping between old and new list elements
	//     + c_ij(value) = min(|v_i - v_j|/Rv,1)
	//     + c_ij(name) = levenshtein, cosine n-grams, jaro-winkler
	//     + c_ij(position_in_list) = min(|p_i - p_j|/Rp, 1)
	// find best mapping between lists
	//     + c_ij(name) = levenshtein, cosine n-grams, jaro-winkler
	//     + c_ij(entries) = entries existing in both
	// 1. h_entry_map = high confidents mapping of list elements (value, name, position_in_list)
	// 2. h_list_map = high confident mapping of lists (name, h_entry_map)
	// 3. entry_map = mapping of list elements (value, name, position_in_list,
	// h_list_map[list_name])
	// 4. list_map = mapping of lists (name, entry_map)


	// high confidance list value matches
	constexpr float HIGH_CONFIDANCE_DROP_PANELTY = 0.3;
	constexpr float LOW_CONFIDANCE_DROP_PANELTY  = 0.6;
	float*          value_matrix                 = cost_matrix(
      MatchListValues{_flag_names.data(), _flag_values.data(), numFlags()},
      MatchListValues{
          old_ref_table._flag_names.data(), old_ref_table._flag_values.data(),
          old_ref_table.numFlags()
      },
      LOW_CONFIDANCE_DROP_PANELTY
  );
	const int   n_flags       = std::max(numFlags(), old_ref_table.numFlags());
	int*        value_matches = new int[n_flags];
	const float value_cost_high
	    = hungarian_solver(value_matrix, value_matches, n_flags, HIGH_CONFIDANCE_DROP_PANELTY);

	// list matches
	float* list_matrix = cost_matrix(
	    MatchList{_list_end.data(), _list_names.data(), numLists()},
	    MatchList{
	        old_ref_table._list_end.data(), old_ref_table._list_names.data(), old_ref_table.numLists()
	    },
	    value_matches, LOW_CONFIDANCE_DROP_PANELTY
	);
	const int   n_lists      = std::max(numLists(), old_ref_table.numLists());
	int*        list_matches = new int[n_lists];
	const float total_list_cost
	    = hungarian_solver(list_matrix, list_matches, n_lists, LOW_CONFIDANCE_DROP_PANELTY);

	// low confidence list_value matches
	const float value_cost_low
	    = hungarian_solver(value_matrix, value_matches, n_flags, LOW_CONFIDANCE_DROP_PANELTY);

	for (size_t idx = 0; idx < old_ref_table._entry_state.size(); ++idx) {
		// migrate
		list new_list{-1};
		switch (old_ref_table._entry_state[idx]) {
			// permanent list are a result of the list definition and do not need to be migrated
			case state::used: new_list = create_at(idx); break;
			default: continue;
		}
		inkAssert(new_list.lid >= 0, "Failed to create new list entry for migration.");
		inkAssert(
		    static_cast<size_t>(new_list.lid) != idx,
		    "At position list creation failed with different valid idx."
		);
		const data_t* entry     = old_ref_table.getPtr(idx);
		data_t*       new_entry = getPtr(idx);
		bool          migrated  = false;
		for (size_t i = 0; i < old_ref_table.numLists(); ++i) {
			if (old_ref_table.hasList(entry, i)) {
				bool hit = false;
				for (size_t j = old_ref_table.listBegin(i); j < old_ref_table._list_end[j]; ++j) {
					if (old_ref_table.hasFlag(entry, j) && old_ref_table._flag_names[j]) {
						if (value_matches[j] != -1) {
							hit      = true;
							migrated = true;
							for (size_t k = 0; _list_end[k] < static_cast<size_t>(value_matches[j]); ++k) {
								setList(new_entry, k);
							}
							setFlag(new_entry, value_matches[j]);
						}
					}
				}
				// keep list if list has match but all values where dropped
				if (! hit && list_matches[i] != -1) {
					setList(new_entry, list_matches[i]);
					migrated = true;
				}
			}
		}
		// drop list
		if (! migrated) {
			// FIXME: remove list ?
			// _entry_state [idx] = state::empty;
			return false;
		}
		// FIXME: use Assert instead?
		// inkAssert(migrated, "Migrating list @%d would lead to an empty list", idx);
	}


	delete[] list_matches;
	delete[] value_matches;
	delete[] value_matrix;
	delete[] list_matrix;
	return true;
}


} // namespace ink::runtime::internal
