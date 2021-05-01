#pragma once

#include "system.h"
#include "array.h"


namespace ink::runtime::internal
{
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
		class list{
			friend class list_table;
			explicit list(int id) : lid{id} {}
			int lid; ///< id of list to handle
		};
		/// handle for an single list flag
		class entry {
			friend class list_table;
			int lid; ///< id of list
			int flag; ///< value of flag
		public:
			explicit entry(int lid, int flag) : lid{lid}, flag{flag} {}
		};

		~list_table();

		/// creates an empty list
		list create();

		/** creates an entry which contains a flag
		 *  of an given list
		 *  @param list_id of list 
		 *  @param flag value in list
		 */
		entry create(size_t list_id, size_t flag);

		/// zeros all usage values
		void clear_usage();

		/// mark list as used
		void mark_used(list);

		/// delete unused lists
		void gc();

		// function to setup list_table
		list_table(const int* list_len, int num_lists);
		list create_permament();
		list& add_inplace(list& lh, entry rh);

		/** set name for an flag
		 * @param lid id of list(type) to set list
		 * @param fid value of flag for this type
		 */
		void setFlagName(entry e, const char* name);
		size_t stringLen(const entry& e) const;
		const char* toString(const entry& e) const;

		/** returns len of string representation of list */
		size_t stringLen(const list& l) const;

		/** converts list to string representation
		 * @param out char array with minimu size of stringLen(l)
		 * @param l list to stringify
		 * @return pointer to end of insierted string
		 */
		char* toString(char* out, const list& l) const;
		
		list add(list lh, list rh);
		list add(list l, int i);
		list sub(list lh, list rh);
		list sub(list l, int i);
		int count(list l);
		entry min(list l);
		entry max(list l);
		entry lrnd(list l);
		list all(list l);
		list invert(list l);
		bool less(list lh, list rh);
		bool greater(list lh, list rh);
		bool equal(list lh, list rh);
		bool not_equal(list lh, list rh){ return equal(lh, rh); }
		bool greater_equal(list lh, list rh);
		bool less_equal(list lh, list rh);
	private:
		void copy_lists(const data_t* src, data_t* dst);
		static constexpr int bits_per_data = sizeof(data_t) * 8;
		int listBegin(int lid) const {
			return lid == 0 ? 0 : _list_end[lid-1];
		}
		const data_t* getPtr(int eid) const {
			return _data.begin();
		}
		data_t* getPtr(int eid)	 {
			return _data.begin() + _entrySize * eid;
		}
		int numFlags() const {
			return _list_end.end()[-1];
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
			setBit(data, lid, value);
		}
		bool hasFlag(const data_t* data, int fid) const {
			return getBit(data, fid + numLists());
		}
		void setFlag(data_t* data, int fid, bool value = true) {
			setBit(data, fid + numLists(), value);
		}
		int toFid(entry e) const;
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
	};
}
