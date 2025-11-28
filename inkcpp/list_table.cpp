/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "list_table.h"
#include "config.h"
#include "system.h"
#include "traits.h"
#include "header.h"
#include "random.h"
#include "string_utils.h"
#include "list_impl.h"

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
	// TODO: initelized unused?
	_entry_state.push() = state::used;
	for (int i = 0; i < _entrySize; ++i) {
		_data.push() = 0;
	}
	return new_entry;
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

} // namespace ink::runtime::internal
