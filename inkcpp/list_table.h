#pragma once

#include "system.h"

namespace ink::runtime::internal {
	using handel_t = uint32_t;

	class list_table;
	struct list_element;
	class list_flags {
	public:
		list_flags(const list_table& table, handel_t list);
		class iterator {
			handel_t operator*() const;
			bool operator==(const iterator& rh) const;
			iterator& operator++();
		};
		iterator begin() const;
		iterator end() const;
	private:
		handel_t _list;
		const list_table& _table;
	};

	class list_table {
	public:
		list_table(size_t num_lists, size_t* list_lengths);
		list_flags get(handel_t list) const;
		// get list_id as input and returns list_id of result list
		handel_t add(handel_t lh, handel_t rh);
		handel_t add(handel_t list, list_element el);
		handel_t subtract(handel_t lh, handel_t rh);
		handel_t subtract(handel_t list, list_element el);
		handel_t inc(handel_t list);
		handel_t dec(handel_t list);
		handel_t count(handel_t list);
		list_element min(handel_t list);
		list_element max(handel_t list);
		handel_t all(handel_t list);
		handel_t invert(handel_t list);

		list_element lrnd(handel_t list);

		// list_id of lh and rh, returns result
		bool is_equal(handel_t lh, handel_t rh);
		bool less_then(handel_t lh, handel_t rh);
		bool greater(handel_t lh, handel_t rh);
		bool less_equal(handel_t lh, handel_t rh);
		bool greater_equal(handel_t lh, handel_t rh);

	private:
		size_t* _list_start; //< maps list id -> first entry
		uint32_t* _start_masks;
		uint32_t* _end_masks;
		uint32_t _entries;
	};
}
