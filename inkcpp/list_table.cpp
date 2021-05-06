#include "list_table.h"
#include "traits.h"
#include "header.h"
#include <iostream>

namespace ink::runtime::internal
{

	void  list_table::copy_lists(const data_t* src, data_t* dst) {
		int len = numLists() / bits_per_data;
		int rest = numLists() / bits_per_data;
		for(int i = 0; i < len; ++i) {
			dst[i] = src[i];
		}
		if (rest) {
			dst[len] |= src[len] & (~static_cast<data_t>(0) << (bits_per_data - rest));
		}
	}

	list_table::list_table(const char* data, const ink::internal::header& header)
		: _valid{false}
	{
		if (data == nullptr) { return; }
		list_flag flag;
		const char* ptr = data;
		int start = 0;
		while((flag = header.read_list_flag(ptr)) != null_flag) {
			if (_list_end.size() == flag.list_id) {
				start = _list_end.size() == 0 ? 0 : _list_end.back();
				_list_end.push() = start;
			}
			while(_list_end.back() - start < flag.flag) {
				_flag_names.push() = nullptr;
				++_list_end.back();
			}
			_flag_names.push() = ptr;
			++_list_end.back();
			while(*ptr) {
				++ptr;
			}
			++ptr;
		}
		_entrySize = segmentsFromBits(
				_list_end.size() + _flag_names.size(),
				sizeof(data_t));
		_valid = true;
	}

	list_table::list list_table::create()
	{
		for(int i = 0; i < _entry_state.size(); ++i) {
			if (_entry_state[i] == state::empty) {
				_entry_state[i] = state::used;
				return list(i);
			}
		}

		list new_entry(_entry_state.size());
		// TODO: initelized unused?
		_entry_state.push() = state::used;
		for(int i = 0; i < _entrySize; ++i) {
			_data.push() = 0;
		}
		return new_entry;
	}


	void list_table::clear_usage() {
		for(state& s : _entry_state) {
			if(s == state::used) {
				s = state::unused;
			}
		}
	}

	void list_table::mark_used(list l) {
		_entry_state[l.lid] = state::used;
	}

	void list_table::gc() {
		for(int i = 0; i < _entry_state.size(); ++i) {
			if (_entry_state[i] == state::unused) {
				_entry_state[i] = state::empty;
				data_t* entry = getPtr(i);
				for(int j = 0; j != _entrySize; ++j) {
					entry[j] = 0;
				}
			}
		}
	}

	int list_table::toFid(list_flag e) const {
		return listBegin(e.list_id) + e.flag;
	}


	size_t list_table::stringLen(const list_flag& e) const {
		return c_str_len(toString(e));
	}
	const char*  list_table::toString(const list_flag &e) const {
		if(e.list_id < 0 || e.flag < 0 ) {
			return nullptr;
		}
		return _flag_names[toFid(e)];
	}

	size_t list_table::stringLen(const list &l) const {
		size_t len = 0;
		len += 2; // '[ '
		const data_t* entry = getPtr(l.lid);
		bool first = true;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(entry, i)) {
				for(int j = listBegin(i); j < _list_end[i]; ++j) {
					if (hasFlag(entry,j)) {
						if(!first) {
							len += 2; // ', '	
						} else {first = false;}
						len += c_str_len(_flag_names[j]);
					}
				}
			}
		}
		len += 2; // ' ]'
		return len;
	}

	char* list_table::toString(char* out, const list& l) const {
		char* itr = out;
		*itr++ = '['; *itr++ = ' ';

		const data_t* entry = getPtr(l.lid);
		bool first = true;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(entry, i)) {
				for(int j = listBegin(i); j < _list_end[i]; ++j) {
					if(hasFlag(entry, j)) {
						if(!first) {
							*itr++ = ','; *itr++ = ' ';
						} else { first = false; }
						for(const char* c = _flag_names[j]; *c; ++c) {
							*itr++ = *c;
						}
					}
				}
			}
		}
		*itr++ = ' '; *itr++ = ']';
		return itr;
	}

	list_table::list list_table::add(list lh, list rh) {
		list res = create();
		data_t* l = getPtr(lh.lid);
		data_t* r = getPtr(rh.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < _entrySize; ++i) {
			o[i] = l[i] & r[i];
		}
		return res;
	}

	list_table::list list_table::create_permament()
	{
		list res = create();
		_entry_state[res.lid] = state::permanent;
		return res;
	}

	list_table::list& list_table::add_inplace(list& lh, list_flag rh) {
		data_t* l = getPtr(lh.lid);
		setList(l, rh.list_id);
		if(rh.flag >= 0) {  // origin entry
			setFlag(l, toFid(rh));
		}
		return lh;
	}

	list_table::list list_table::add(list lh, list_flag rh) {
		list res = create();
		data_t* l = getPtr(lh.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < _entrySize; ++i) {
			o[i] = l[i];
		}
		setList(o, rh.list_id);
		setFlag(o, toFid(rh));
		return res;
	}
	
	list_table::list list_table::sub(list lh, list rh) {
		list res = create();
		data_t* l = getPtr(lh.lid);
		data_t* r = getPtr(rh.lid);
		data_t* o = getPtr(res.lid);
		bool active_flag = false;
		for(int i = 0; i < _entrySize; ++i) {
			o[i] = (l[i] & r[i]) ^ l[i];
		}

		for(int i = 0; i < numLists(); ++i) {
			if (hasList(r,i)) {
				if (hasList(l,i)) {
					for(int j = listBegin(i); j < _list_end[j]; ++j)
					{
						if(hasFlag(o, j)) {
							setList(o,i);
							active_flag = true;
							break;
						}
					}
				}
			}
		}
		if(active_flag) { return res; }
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(o,i)) {
				return res;
			}
		}
		copy_lists(l, o);
		return res;
	}

	list_table::list list_table::sub(list lh, list_flag rh) {
		list res = create();
		data_t* l = getPtr(lh.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < numLists(); ++i) {
			o[i] = l[i];
		}
		setFlag(o, toFid(rh), false);
		for(int i = listBegin(rh.list_id); i < _list_end[rh.list_id]; ++i) {
			if(hasFlag(o, i)) {
				return res;
			}
		}
		setList(l, rh.list_id, false);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(o,i)) {
				return res;
			}
		}
		copy_lists(l, o);
		return res;
	}


	list_table::list list_table::add(list arg, int i) {
		list res = create();
		data_t* l = getPtr(arg.lid);
		data_t* o = getPtr(res.lid);
		bool active_flag = false;;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l, i)) {
				bool has_flag = false;
				for(int j = listBegin(i); j < _list_end[i] - i;++j)
				{
					if(hasFlag(l, j)) {
						setFlag(o,j+i);
						has_flag = true;
					}
				}
				if(has_flag) {
					active_flag = true;
					setList(o,i);
				}
			}
		}
		if(!active_flag) {
			copy_lists(l, o);
		}
		return res;
	}

	list_table::list list_table::sub(list arg, int i) {
		list res = create();
		data_t* l = getPtr(arg.lid);
		data_t* o = getPtr(res.lid);
		bool active_flag = false;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l, i)) {
				bool has_flag = false;
				for(int j = listBegin(i) + i; j < _list_end[i]; ++j)
				{
					if(hasFlag(l,j)) {
						setFlag(o,j-i);
						has_flag = true;
					}
				}
				if(has_flag) {
					active_flag = true;
					setList(o, i);
				}
			}
		}
		if (!active_flag) {
			copy_lists(l, o);
		}
		return res;
	}

	int list_table::count(list l) {
		int count = 0;
		data_t* data = getPtr(l.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(data, i)) {
				for(int j = listBegin(i); j < _list_end[j]; ++j)
				{
					if(hasFlag(data, j)) {
						++count;
					}
				}
			}
		}
		return count;
	}

	list_flag list_table::min(list l) {
		list_flag res{-1,-1};
		data_t* data = getPtr(l.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(data, i)) {
				for(int j = listBegin(i); j < _list_end[j]; ++j) {
					if(hasFlag(data, j)) {
						int value = j - listBegin(i);
						if(res.flag < 0 || value < res.flag) {
							res.flag = value;
							res.list_id = i;
						}
						break;
					}
				}
			}
		}
		return res;
	}

	list_flag list_table::max(list l) {
		list_flag res{-1,-1};
		data_t* data = getPtr(l.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(data, i)) {
				for(int j = _list_end[j] - 1; j >= listBegin(i); --j)
				{
					if(hasFlag(data, j)) {
						int value = j - listBegin(i);
						if (value > res.flag) {
							res.flag = value;
							res.list_id = i;
						}
						break;
					}
				}
			}
		}
		return res;
	}

	list_table::list list_table::all(list arg) {
		list res = create();
		data_t* l = getPtr(arg.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l, i)) {
				setList(o,i);
				for(int j = listBegin(i); i < _list_end[i]; ++j)
				{
					setBit(o, j);
				}
			}
		}
		return res;
	}

	// ATTENTION: can produce an list without setted flag list
	list_table::list list_table::invert(list arg) {
		list res = create();
		data_t* l = getPtr(arg.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l, i)) {
				setList(o,i);
				for(int j = listBegin(i); i < _list_end[i]; ++j)
				{
					setBit(o, j, !getBit(l,j));
				}
			}
		}
		return res;
	}
}

