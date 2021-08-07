#pragma once

#include "system.h"
#include "array.h"

#ifdef INK_ENABLE_STL
#include <iosfwd>
#endif

namespace ink::internal {
	class header;
}
namespace ink::runtime::internal
{
	class prng;

	// TODO: move to utils
	// memory segments
	// @param bits size in bits
	// @param size segment size in bytes
	constexpr int segmentsFromBits(int bits, int size) {
		size *= 8;
		return bits / size + (bits % size ? 1 : 0);
	}

	/// managed all list entries and list metadata
	class list_table
	{
		using data_t = int;
		enum class state : char {
			unused,
			used,
			permanent,
			empty
		};

	public:
		/// handle to acces a list
		struct list{
			constexpr explicit list(int id) : lid{id} {}
			int lid; ///< id of list to handle
		};

		/// creates an empty list
		list create();
		
		/** @return list_flag with list_id set to list with name list_name */
		list_flag get_list_id(const char* list_name) const;

		/// zeros all usage values
		void clear_usage();

		/// mark list as used
		void mark_used(list);

		/// delete unused lists
		void gc();


		// function to setup list_table
		list create_permament();
		list& add_inplace(list& lh, list_flag rh);

		// parse binary list meta data
		list_table(const char* data, const ink::internal::header&);
		explicit list_table() : _valid{false} {}
		size_t stringLen(const list_flag& e) const;
		const char* toString(const list_flag& e) const;

		/** returns len of string representation of list */
		size_t stringLen(const list& l) const;

		/** converts list to string representation
		 * @param out char array with minimu size of stringLen(l)
		 * @param l list to stringify
		 * @return pointer to end of insierted string
		 */
		char* toString(char* out, const list& l) const;

		/** special traitment when a list get assignet again
		 * when a list get assigned and would have no origin, it gets the origin of the base with origin
		 * eg. I072
		 */
		list redefine(list lh, list rh);
		
		list add(list l, int i);
		list_flag add(list_flag f, int i);

		list add(list lh, list rh);
		list add(list lh ,list_flag rh);
		list add(list_flag lh, list rh) { return add(rh, lh); }
		list add(list_flag lh, list_flag rh);

		list sub(list l, int i);
		list_flag sub(list_flag l, int i);
		list sub(list lh, list rh);
		list sub(list lh, list_flag rh);
		list_flag sub(list_flag lh, list rh);
		list_flag sub(list_flag lh, list_flag rh) {
			return lh == rh ? list_flag{lh.list_id, -1} : lh;
		}
		list intersect(list lh, list rh);
		list_flag intersect(list lh, list_flag rh);
		list_flag intersect(list_flag lh, list rh) { return intersect(rh, lh); }
		list_flag intersect(list_flag lh, list_flag rh) {
			return lh == rh ? lh : null_flag;
		}
		int count(list l) const;
		int count(list_flag f) const { return f.flag < 0 ? 0 : 1; }
		list_flag min(list l) const;
		list_flag min(list_flag f) const { return f; }
		list_flag max(list l) const;
		list_flag max(list_flag f) const { return f; }
		list_flag lrnd(list l, prng&) const;
		list_flag lrnd(list_flag f) const { return f; }
		list all(list l);
		list all(list_flag l);
		list invert(list l);
		list invert(list_flag f);
		template<typename L, typename R>
		bool less(L lh, R rh) const { return max(lh).flag < min(rh).flag; }
		template<typename L, typename R>
		bool greater(L lh, R rh) const { return min(lh).flag > max(rh).flag; }
		bool equal(list lh, list rh) const;
		bool equal(list lh, list_flag rh) const;
		bool equal(list_flag lh, list rh) const { return equal(rh, lh); }
		bool equal(list_flag lh, list_flag rh) const { return lh == rh; }
		template<typename L, typename R>
		bool not_equal(L lh, R rh) const { return equal(lh, rh); }
		template<typename L, typename R>
		bool greater_equal(L lh, R rh) const {
			return max(lh).flag >= max(rh).flag && min(lh).flag >= min(rh).flag;
		}
		template<typename L, typename R>
		bool less_equal(L lh, R rh) const {
			return max(lh).flag <= max(rh).flag && min(lh).flag <= min(rh).flag;
		}
		bool has(list lh, list rh) const;
		bool has(list lh, list_flag rh) const;
		bool has(list_flag lh, list rh) const { return has(rh, lh); } 
		bool has(list_flag lh, list_flag rh) const { return lh == rh; }
		template<typename L, typename R>
		bool hasnt(L lh, R rh) const { return !has(lh,rh); }
		operator bool () const{
			return _valid;
		}
		list range(list l, int min, int max);

	private:
		void copy_lists(const data_t* src, data_t* dst);
		static constexpr int bits_per_data = sizeof(data_t) * 8;
		int listBegin(int lid) const {
			return lid == 0 ? 0 : _list_end[lid-1];
		}
		const data_t* getPtr(int eid) const {
			return _data.begin() + _entrySize * eid;
		}
		data_t* getPtr(int eid)	 {
			return _data.begin() + _entrySize * eid;
		}
		int numFlags() const {
			return _flag_names.size();
			// return _list_end.end()[-1]; TODO: 
		}
		int numLists() const {
			return _list_end.size();
		}
		bool getBit(const data_t* data, int id) const {
			return data[id / bits_per_data] &
				(0x01 << (bits_per_data - 1 - (id % bits_per_data)));
		}
		void setBit(data_t* data, int id, bool value = true) {
			data_t mask = 0x01 << (bits_per_data-1 - (id % bits_per_data));
			if (value) {
				data[id/bits_per_data] |= mask;
			} else {
				data[id/bits_per_data] &= ~mask;
			}
		}
		bool hasList(const data_t* data, int lid) const {
			return getBit(data, lid);
		}
		void setList(data_t* data, int lid, bool value = true) {
			if(lid >= 0) {
				setBit(data, lid, value);
			}
		}
		bool hasFlag(const data_t* data, int fid) const {
			return getBit(data, fid + numLists());
		}
		void setFlag(data_t* data, int fid, bool value = true) {
			if (fid >= 0) {
				setBit(data, fid + numLists(), value);
			}
		}
		int toFid(list_flag e) const;
		auto flagStartMask() const {
			struct { int segment; data_t mask; }
			res {
				numLists() / bits_per_data,
				~static_cast<data_t>(0) >> (numLists() % bits_per_data)};
			return res;
		}

		template<typename T, int config>
		using managed_array = managed_array<T, config < 0, abs(config)>;

		static constexpr int maxMemorySize = 
			(config::maxListTypes < 0
			 || config::maxFlags < 0
			 || config::maxLists < 0
			 ? -1
			 : 1) *
			segmentsFromBits(
					abs(config::maxListTypes)
					+ abs(config::maxFlags),
					sizeof(data_t)
			) * abs(config::maxLists);

		int _entrySize; ///< entry size in data_t 
		// entries (created lists)
		managed_array<data_t, maxMemorySize> _data;
		managed_array<state, config::maxLists> _entry_state;

		// defined list (meta data)
		managed_array<int, config::maxListTypes> _list_end;
		managed_array<const char*,config::maxFlags> _flag_names;
		managed_array<const char*,config::maxListTypes> _list_names;

		bool _valid;
	public:
		friend class name_flag_itr;
		class named_flag_itr {
			const list_table& _list;
			const data_t* _data;
			struct {
				list_flag flag;
				const char* name;
			} _pos;
			void carry() {
				if (_pos.flag.flag == 
						_list._list_end[_pos.flag.list_id] - _list.listBegin(_pos.flag.list_id)) {
					_pos.flag.flag = 0;
					++_pos.flag.list_id;
				}
				if(_pos.flag.list_id == _list.numLists()) {
					_pos.flag = null_flag;
				} else {
					_pos.name = _list._flag_names[_list.toFid(_pos.flag)];
				}
			}
			void goToValid() {
				bool valid;
				do { 
					valid = true;
					int fid = _list.toFid(_pos.flag);
					if (_data == nullptr) {
						if(_list._flag_names[fid] == nullptr) {
							valid = false;
							++_pos.flag.flag;
						}
					} else if(!_list.hasList(_data, _pos.flag.list_id)) {
						valid = false;
						++_pos.flag.list_id;
					} else if (!_list.hasFlag(_data, fid) || _list._flag_names[fid] == nullptr) {
						valid = false;
						++_pos.flag.flag;
					}			
					carry();
				} while(_pos.flag != null_flag && !valid);
			}
		public:
			bool operator!=(const named_flag_itr& o) const  {
				return _pos.flag != o._pos.flag;
			}
			named_flag_itr(const list_table& list, const data_t* filter)
				: _list{list}, _data{filter}, _pos{null_flag, nullptr} {};
			named_flag_itr(const list_table& list, const data_t* filter, int)
				: _list{list}, _data{filter}, _pos{{0,0},list._flag_names[0]}{
					goToValid();
			}
			const auto* operator->() const { return &_pos; }
			const auto& operator*() const { return _pos; }
			const named_flag_itr& operator++() {
				if (_pos.flag == null_flag) return *this;
				++_pos.flag.flag;
				carry();
				if (_pos.flag == null_flag) return *this;
				goToValid();
				return *this;
			}
		};
		auto named_flags(list filter = list(-1)) const {
			const data_t* f = filter.lid < 0 ? nullptr : getPtr(filter.lid);
			struct { named_flag_itr _begin; named_flag_itr _end;
					named_flag_itr begin() const { return _begin; }
					named_flag_itr end() const { return _end; }
			} res {
				named_flag_itr(*this, f, 0),
				named_flag_itr(*this, f)};
			return res;
		}
#ifdef INK_ENABLE_STL
		std::ostream& write(std::ostream&,list) const;
#endif
	};
}
