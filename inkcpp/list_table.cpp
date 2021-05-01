#include "list_table.h"
#include "traits.h"

namespace ink::runtime::internal
{
	list_table::list_table(const int* list_len, int num_lists)
	{
		int sum = 0;
		for (int i = 0; i < num_lists; ++i) {
			sum += list_len[i];
			_list_end.push() = sum;
		}
		_entrySize = segmentsFromBits(
				num_lists + sum,
				sizeof(data_t));
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

	list_table::entry list_table::create(size_t list_id, size_t flag)
	{
		return entry(list_id, flag);
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

	int list_table::toFid(entry e) const {
		return listBegin(e.lid) + e.flag;
	}

	void list_table::setFlagName(entry e, const char* name) {
		int slot = toFid(e);
		while(slot >= _flag_names.size()) {
			_flag_names.push() = nullptr;
		}
		_flag_names[slot] = name;
	}

	size_t list_table::stringLen(const entry& e) const {
		return c_str_len(_flag_names[toFid(e)]);
	}
	const char*  list_table::toString(const entry &e) const {
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

	list_table::list& list_table::add_inplace(list& lh, entry rh) {
		data_t* l = getPtr(lh.lid);
		setList(l, rh.lid);
		setFlag(l, toFid(rh));
		return lh;
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

	list_table::entry list_table::min(list l) {
		entry res(-1,numFlags());
		data_t* data = getPtr(l.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(data, i)) {
				for(int j = listBegin(i); j < _list_end[j]; ++j) {
					if(hasFlag(data, j)) {
						int value = j - listBegin(i);
						if(value < res.flag) {
							res.flag = value;
							res.lid = i;
						}
						break;
					}
				}
			}
		}
		return res;
	}

	list_table::entry list_table::max(list l) {
		entry res(-1,-1);
		data_t* data = getPtr(l.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(data, i)) {
				for(int j = _list_end[j] - 1; j >= listBegin(i); --j)
				{
					if(hasFlag(data, j)) {
						int value = j - listBegin(i);
						if (value > res.flag) {
							res.flag = value;
							res.lid = i;
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

	void list_table::copy_lists(const data_t* src, data_t* dst) {
		data_t mask =
			~static_cast<data_t>(0) << (bits_per_data - (numLists() % bits_per_data));
		int mask_pos = numLists() / bits_per_data;
		for(int i = 0; i < mask_pos; ++i) {
			dst[i] = src[i];
		}
		dst[mask_pos] |= mask & src[mask_pos];
	}
}

