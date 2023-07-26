#pragma once

#include "system.h"

#ifdef INK_ENABLE_STL
#include <ostream>
#endif

namespace ink::runtime {
	namespace internal {
		class list_table;
	}
	class list_interface {
	public:
		list_interface() : _list_table{nullptr}, _list{-1} {}

		class iterator {
			const char* _flag_name;
			const char* _list_name;
			const list_interface& _list;
			int _i;
			bool _one_list_iterator; //< iterates only though values of one list
			friend list_interface;
		protected:
			iterator(const char* flag_name, const list_interface& list, size_t i, bool one_list_only = false)
				: _flag_name(flag_name), _list(list), _i(i), _one_list_iterator(one_list_only) {}
		public:
			struct Flag {
				const char* flag_name;
				const char* list_name;
#ifdef INK_ENABLE_STL
				friend std::ostream& operator<<(std::ostream& os, const Flag& flag) {
					os << flag.list_name << "(" << flag.flag_name << ")";
					return os;
				}
#endif
			};
			Flag      operator*() const { return Flag{ .flag_name = _flag_name, .list_name = _list_name }; };
			iterator& operator++()
			{
				_list.next( _flag_name, _list_name, _i );
				return *this;
			}
			bool operator!=(const iterator& itr) const { return itr._i != _i; }
			bool operator==(const iterator& itr) const {
				return itr._i == _i;
			}
		};

		/** checks if a flag is contained in the list */
		virtual bool contains(const char* flag) const {
			inkAssert(false, "Not implemented function from interfaces is called!"); return false;
		};

		/** add a flag to list */
		virtual void add(const char* flag) {
			inkAssert(false, "Not implemented function from interface is called!");
		};

		/** removes a flag from list */
		virtual void remove(const char* flag) {
			inkAssert(false, "Not implemented function from interface is called!");
		};

		/** begin iterator for contained flags in list */
		virtual iterator begin() const {
			inkAssert(false, "Not implemented function from interface is called!");
			return new_iterator(nullptr, -1);
		};

		/** returns a iterator over elements of the given list */
		virtual iterator begin(const char* list_name) const {
			inkAssert(false, "Not implemented function from interface is called!");
			return new_iterator(nullptr, -1);
		}
		/** end iterator for contained flags in list */
		virtual iterator end() const {
			inkAssert(false, "Not implemented function from interface is called!");
			return new_iterator(nullptr, -1); };

	private:
		friend iterator;
		virtual void next(const char*& flag_name, const char*& list_name, int& i) const {
			inkAssert(false, "Not implemented funciton from interface is called!");
		};
	protected:
		iterator new_iterator(const char* flag_name, int i, bool one_list_only = false) const {
			return iterator(flag_name, *this, i, one_list_only);
		}
		list_interface(internal::list_table& table, int list) : _list_table {&table}, _list{list} {}
    internal::list_table* _list_table;
    int _list;
	};
}
