#pragma once

#include "system.h"

namespace ink::runtime {
	namespace internal {
		class list_table;
	}
	class list_interface {
	public:
		list_interface() : _list_table{nullptr}, _list{-1} {}

		class iterator {
			const char* _flag_name;
			const list_interface& _list;
			int _i;
			friend list_interface;
		protected:
			iterator(const char* flag_name, const list_interface& list, size_t i)
				: _flag_name(flag_name), _list(list), _i(i) {}
		public:
			const char* operator*() const { return _flag_name; };
			iterator& operator++() { _list.next(_flag_name, _i); return *this; }
			bool operator!=(const iterator& itr) const { return itr._i != _i; }
			bool operator==(const iterator& itr) const {
				return itr._i == _i;
			}
		};

		/** checks if a flag is contained in the list */
		virtual bool contains(const char* flag) const {
			ink_assert(false, "Not implemented function from interfaces is called!"); return false;
		};

		/** add a flag to list */
		virtual void add(const char* flag) {
			ink_assert(false, "Not implemented function from interface is called!");
		};

		/** removes a flag from list */
		virtual void remove(const char* flag) {
			ink_assert(false, "Not implemented function from interface is called!");
		};

		/** begin iterator for contained flags in list */
		virtual iterator begin() const {
			ink_assert(false, "Not implemented function from interface is called!");
			return new_iterator(nullptr, -1);
		};
		/** end iterator for contained flags in list */
		virtual iterator end() const {
			ink_assert(false, "Not implemented function from interface is called!");
			return new_iterator(nullptr, -1); };

	private:
		friend iterator;
		virtual void next(const char*& flag_name, int& i) const {
			ink_assert(false, "Not implemented funciton from interface is called!");
		};
	protected:
		iterator new_iterator(const char* flag_name, int i) const {
			return iterator(flag_name, *this, i);
		}
		list_interface(internal::list_table& table, int list) : _list_table {&table}, _list{list} {}
    internal::list_table* _list_table;
    int _list;
	};
}
