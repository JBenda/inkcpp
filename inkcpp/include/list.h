#pragma once

#include "system.h"

#ifdef INK_ENABLE_STL
#	include <ostream>
#endif

#ifdef INK_BUILD_CLIB
struct InkListIter;
struct HInkList;
int ink_list_flags(const HInkList*, InkListIter*);
int ink_list_flags_from(const HInkList*, const char*, InkListIter*);
int ink_list_iter_next(InkListIter*);
#endif

namespace ink::runtime
{
namespace internal
{
	class list_table;
} // namespace internal

/** Interface for accessing a Ink list.
 *
 * Every function which takes a flag name can also be feed with
 * a full flag name in the format `listName.flagName`
 * @code
 * using namespace ink::runtime;
 * list l = globals.get<list>("list_var");
 * l.add("flagName");
 * l.add("listName.FlagName")
 * globals.set("list_var", l);
 * @endcode
 */
class list_interface
{
public:
	list_interface()
	    : _list_table{nullptr}
	    , _list{-1}
	{
	}

	/** iterater for flags in a list
	 * @todo implement `operator->`
	 */
	class iterator
	{
		const char*           _flag_name;
		const char*           _list_name;
		const list_interface& _list;
		int                   _i;
		bool                  _one_list_iterator; ///< iterates only though values of one list
		friend list_interface;
#ifdef INK_BUILD_CLIB
		friend int ::ink_list_flags(const HInkList*, InkListIter*);
		friend int ::ink_list_flags_from(const HInkList*, const char*, InkListIter*);
		friend int ::ink_list_iter_next(InkListIter* self);
#endif

	protected:
		/** @private */
		iterator(
		    const char* flag_name, const list_interface& list, size_t i, bool one_list_only = false
		)
		    : _flag_name(flag_name)
		    , _list(list)
		    , _i(i)
		    , _one_list_iterator(one_list_only)
		{
		}

	public:
		/** contains flag data */
		struct Flag {
			const char* flag_name; ///< name of the flag
			const char* list_name; ///< name of the list
#ifdef INK_ENABLE_STL
			/** serelization operator
			 * @param os
			 * @param flag
			 */
			friend std::ostream& operator<<(std::ostream& os, const Flag& flag)
			{
				os << flag.list_name << "(" << flag.flag_name << ")";
				return os;
			}
#endif
		};

		/** access value the iterator is pointing to */
		Flag operator*() const { return Flag{.flag_name = _flag_name, .list_name = _list_name}; };

		/** continue iterator to next value */
		iterator& operator++()
		{
			_list.next(_flag_name, _list_name, _i, _one_list_iterator);
			return *this;
		}

		/** checks if iterator points not to the same element
		 * @param itr other iterator
		 */
		bool operator!=(const iterator& itr) const { return itr._i != _i; }

		/** checks if iterator points to the same element
		 * @param itr other iterator
		 */
		bool operator==(const iterator& itr) const { return itr._i == _i; }
	};

	/** checks if a flag is contained in the list */
	virtual bool contains(const char* flag) const
	{
		inkAssert(false, "Not implemented function from interfaces is called!");
		return false;
	};

	/** add a flag to list */
	virtual void add(const char* flag)
	{
		inkAssert(false, "Not implemented function from interface is called!");
	};

	/** removes a flag from list */
	virtual void remove(const char* flag)
	{
		inkAssert(false, "Not implemented function from interface is called!");
	};

	/** begin iterator for contained flags in list */
	virtual iterator begin() const
	{
		inkAssert(false, "Not implemented function from interface is called!");
		return new_iterator(nullptr, -1);
	};

	/** returns a iterator over elements of the given list */
	virtual iterator begin(const char* list_name) const
	{
		inkAssert(false, "Not implemented function from interface is called!");
		return new_iterator(nullptr, -1);
	}

	/** end iterator for contained flags in list */
	virtual iterator end() const
	{
		inkAssert(false, "Not implemented function from interface is called!");
		return new_iterator(nullptr, -1);
	};

private:
	friend iterator;

	virtual void
	    next(const char*& flag_name, const char*& list_name, int& i, bool _one_list_iterator) const
	{
		inkAssert(false, "Not implemented funciton from interface is called!");
	};

protected:
	/** @private */
	iterator new_iterator(const char* flag_name, int i, bool one_list_only = false) const
	{
		return iterator(flag_name, *this, i, one_list_only);
	}

	/** @private */
	list_interface(internal::list_table& table, int list)
	    : _list_table{&table}
	    , _list{list}
	{
	}

	/** @private */
	internal::list_table* _list_table;
	/** @private */
	int                   _list;
};
} // namespace ink::runtime
