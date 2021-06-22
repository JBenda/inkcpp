#include "list_table.h"
#include "traits.h"
#include "header.h"
#include "random.h"
#include "string_utils.h"

#ifdef INK_ENABLE_STL
#include <ostream>
#endif

namespace ink::runtime::internal
{

	void  list_table::copy_lists(const data_t* src, data_t* dst) {
		int len = numLists() / bits_per_data;
		int rest = numLists() % bits_per_data;
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
				_list_names.push() = ptr;
				while(*ptr) { ++ptr; } ++ptr; // skip string
			}
			while(_list_end.back() - start < flag.flag) {
				_flag_names.push() = nullptr;
				++_list_end.back();
			}
			_flag_names.push() = ptr;
			++_list_end.back();
			while(*ptr) { ++ptr; } ++ptr; // skip string
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
		const data_t* entry = getPtr(l.lid);
		bool first = true;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(entry, i)) {
				for(int j = listBegin(i); j < _list_end[i]; ++j) {
					if (hasFlag(entry,j) && _flag_names[j]) {
						if(!first) {
							len += 2; // ', '	
						} else {first = false;}
						len += c_str_len(_flag_names[j]);
					}
				}
			}
		}
		return len;
	}

	char* list_table::toString(char* out, const list& l) const {
		char* itr = out;

		const data_t* entry = getPtr(l.lid);
		bool first = true;
		int max_list_len = 0;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(entry,i)) {
				int len = _list_end[i] - listBegin(i);
				if (len > max_list_len) max_list_len = len;
			}
		}
		for(int j = 0; j < max_list_len; ++j) {
			for(int i = 0; i < numLists(); ++i) {
				int len = _list_end[i] - listBegin(i);
				if(j < len && hasList(entry, i)) {
					int flag = j + listBegin(i);
					if(hasFlag(entry,flag) && _flag_names[flag]) {
						if(!first) {
							*itr++ = ','; *itr++ = ' ';
						} else { first = false; }
						for(const char* c = _flag_names[flag]; *c; ++c) {
							*itr++ = *c;
						}
					}
				}
			}
		}
		return itr;
	}

	list_table::list list_table::range(list_table::list l, int min, int max) {
		list res = create();
		data_t* in = getPtr(l.lid);
		data_t* out = getPtr(res.lid);
		bool has_any_list = false;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(in, i)) {
				bool has_flag = false;
				for(int j = listBegin(i); j < _list_end[i]; ++j) {
					if(j - listBegin(i) < min || j - listBegin(i) > max) { continue; }
					if(hasFlag(in, j)) {
						setFlag(out,j);
						has_flag = true;
					}
				}
				if(has_flag) {
					has_any_list = true;
					setList(out, i);
				}
			}
		}
		if(has_any_list) { return res; }
		copy_lists(in, out);
		return res;
	}

	list_table::list list_table::add(list_flag lh, list_flag rh) {
		list res = create();
		data_t* o = getPtr(res.lid);
		setList(o, lh.list_id);
		setFlag(o, toFid(lh));
		setList(o, rh.list_id);
		setFlag(o, toFid(rh));
		return res;
	}
	list_table::list list_table::add(list lh, list rh) {
		list res = create();
		data_t* l = getPtr(lh.lid);
		data_t* r = getPtr(rh.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < _entrySize; ++i) {
			o[i] = l[i] | r[i];
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
		if(rh.list_id < 0) return lh; // empty or null flag (skip)
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
		for(int i = 0; i < _entrySize; ++i) {
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

	list_flag list_table::sub(list_flag lh, list rh) {
		data_t* r = getPtr(rh.lid);
		if(hasList(r, lh.list_id) && hasFlag(r, toFid(lh))) {
			return list_flag{lh.list_id, -1};
		}
		return lh;
	}


	list_table::list list_table::add(list arg, int i) {
		// TODO: handle i == 0 (for performance only)
		if (i < 0) {
			return sub(arg, -i);
		}
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

	list_flag list_table::add(list_flag arg, int i) {
		arg.flag += i;
		if (arg.flag < 0 || arg.flag > _list_end[arg.list_id] - listBegin(arg.list_id))
		{
			arg.flag = -1;
		}
		return arg;
	}

	list_table::list list_table::sub(list arg, int i) {
		// TODO: handle i == 0 (for perofrgmance only)
		if(i < 0) {
			return add(arg, -i);
		}
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
	list_flag list_table::sub(list_flag arg, int i) {
		arg.flag -= i;
		if (arg.flag < 0 || arg.flag > _list_end[arg.list_id] - listBegin(arg.list_id))
		{
			arg.flag = -1;
		}
		return arg;
	}
	
	int list_table::count(list l) const {
		int count = 0;
		const data_t* data = getPtr(l.lid);
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

	list_flag list_table::min(list l) const {
		list_flag res{-1,-1};
		const data_t* data = getPtr(l.lid);
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

	list_flag list_table::max(list l) const{
		list_flag res{-1,-1};
		const data_t* data = getPtr(l.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(data, i)) {
				for(int j = _list_end[i] - 1; j >= listBegin(i); --j)
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

	bool list_table::equal(list lh, list rh) const {
		const data_t* l = getPtr(lh.lid);
		const data_t* r = getPtr(rh.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l, i) != hasList(r,i)) { return false; }
			if (hasList(l,i)) {
				for(int j = listBegin(i); j < _list_end[i]; ++j) {
					if(hasFlag(l,j) != hasFlag(r,j)) {
						return false;
					}
				}
			}
		}
		return true;
	}

	bool list_table::equal(list lh, list_flag rh) const {
		const data_t* l = getPtr(lh.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l,i) != (rh.list_id == i)) { return false; }
		}
		for(int i = listBegin(rh.list_id); i < _list_end[rh.list_id]; ++i) {
			if(hasFlag(l,i) != (rh.flag == i - listBegin(rh.list_id))) {
				return false;
			}
		}
		return true;
	}


	list_table::list list_table::all(list arg) {
		list res = create();
		data_t* l = getPtr(arg.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l, i)) {
				setList(o,i);
				for(int j = listBegin(i); j < _list_end[i]; ++j)
				{
					setFlag(o, j);
				}
			}
		}
		return res;
	}

	list_table::list list_table::all(list_flag arg) {
		list res = create();
		if(arg != null_flag) {
			data_t* o = getPtr(res.lid);
			setList(o, arg.list_id);
			for(int i = listBegin(arg.list_id); i < _list_end[arg.list_id]; ++i) {
				setFlag(o, i);
			}
		}
		return res;
	}

	// ATTENTION: can produce an list without setted flag list (same behavior than inklecate)
	list_table::list list_table::invert(list arg) {
		list res = create();
		data_t* l = getPtr(arg.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l, i)) {
				bool hasList = false;
				for(int j = listBegin(i); j < _list_end[i]; ++j)
				{
					bool have = hasFlag(l,j);
					if(!have) {
						hasList = true;
						setFlag(o,j);
					}
				}
				if(hasList) {
					setList(o,i);
				}
			}
		}
		return res;
	}

	list_table::list list_table::invert(list_flag arg) {
		list res = create();
		if(arg != null_flag) {
			data_t* o = getPtr(res.lid);
			for(int i = listBegin(arg.list_id); i < _list_end[arg.list_id]; ++i) {
				setFlag(o, i, i - listBegin(arg.list_id) != arg.flag);
			}
		}
		return res;
	}

	list_table::list list_table::intersect(list lh, list rh) {
		list res = create();
		data_t* l = getPtr(lh.lid);
		data_t* r = getPtr(rh.lid);
		data_t* o = getPtr(res.lid);
		for(int i = 0; i < _entrySize; ++i) {
			o[i] = l[i] & r[i];
		}
		return res;
	}

	list_flag list_table::intersect(list lh, list_flag rh) {
		const data_t* l = getPtr(lh.lid);
		if(hasList(l, rh.list_id) && hasFlag(l, toFid(rh))) {
			return rh;
		}
		return null_flag;
	}

	bool list_table::has(list lh, list_flag rh) const {
		const data_t* l = getPtr(lh.lid);
		return hasList(l, rh.list_id) && hasFlag(l, toFid(rh));
	}

	list_flag list_table::lrnd(list lh, prng& rng) const {
		const data_t* l = getPtr(lh.lid);
		int i = count(lh);
		rng.rand(i);		
		int count = 0;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(l, i)) {
				for(int j = listBegin(i); j < _list_end[i]; ++j) {
					if(hasFlag(l,j)) {
						if(count++ == i) {
							return list_flag{
								static_cast<decltype(list_flag::list_id)>(i),
								static_cast<decltype(list_flag::flag)>( j - listBegin(i) )
							};
						}
					}
				}
			}
		}
		return null_flag;
	}

	bool list_table::has(list lh, list rh) const {
		const data_t* r = getPtr(rh.lid);
		const data_t* l = getPtr(lh.lid);
		for(int i = 0; i < numLists(); ++i) {
			if (hasList(r, i)) {
				if(!hasList(l, i)) { return false; }
				for(int j = listBegin(i); j < _list_end[i]; ++j) {
					if(hasFlag(r, j) && !hasFlag(l,j)) { return false; }
				}
			}
		}
		return true;
	}

	list_flag list_table::get_list_id(const char *list_name) const {
		using int_t = decltype(list_flag::list_id);
		for(int_t i = 0; i < static_cast<int_t>(_list_names.size()); ++i) {
			if(str_equal(list_name, _list_names[i])) {
				return list_flag{i, -1};
			}
		}
		inkAssert(false, "No list with name found!");
		return null_flag;
	}

	list_table::list list_table::redefine(list lh, list rh) {
		data_t* l = getPtr(lh.lid);
		data_t* r = getPtr(rh.lid);
		list res = create();
		data_t* o = getPtr(res.lid);

		// if the new list has no origin: give it the origin of the old value
		bool has_origin = false;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(r, i)) { has_origin = true; break; }
		}
		if(!has_origin) {
			copy_lists(l, r);
		}

		for(int i = 0; i < _entrySize; ++i) {
			o[i] = r[i];
		}
		return res;
	}

#ifdef INK_ENABLE_STL
	std::ostream& list_table::write(std::ostream& os, list l) const {
		bool first = true;

		const data_t* entry = getPtr(l.lid);
		int max_list_len = 0;
		for(int i = 0; i < numLists(); ++i) {
			if(hasList(entry,i)) {
				int len = _list_end[i] - listBegin(i);
				if (len > max_list_len) max_list_len = len;
			}
		}
		for(int j = 0; j < max_list_len; ++j) {
			for(int i = 0; i < numLists(); ++i) {
				int len = _list_end[i] - listBegin(i);
				if(j < len && hasList(entry, i)) {
					int flag = listBegin(i) + j;
					if(hasFlag(entry,flag) && _flag_names[flag]) {
						if(!first) {
							os << ", ";
						} else { first = false; }
						os << _flag_names[flag];
					}
				}
			}
		}
		return os;
	}
#endif

}

