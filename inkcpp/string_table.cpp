#include "string_table.h"

namespace ink::runtime::internal
{
	string_table::~string_table()
	{
		// Delete all allocated strings
		for (auto iter = _table.begin(); iter != _table.end(); ++iter)
			delete[] iter.key();
		_table.clear();
	}
	char* string_table::duplicate(const char* str)
	{
		int len = 0;
		for(const char* i = str; *i != 0; ++i) {
			++len;
		}
		char* res = create(len + 1);
		char* out = res;
		for(const char* i = str; *i != 0; ++i, ++out) {
			*out = *i;
		}
		*out = 0;
		return res;
	}

	char* string_table::create(size_t length)
	{
		// allocate the string
		char* data = new char[length];
		if (data == nullptr)
			return nullptr;

		// Add to the tree
		bool success = _table.insert(data, true); // TODO: Should it start as used?
		inkAssert(success, "Duplicate string pointer in the string_table. How is that possible?");
		if (!success)
		{
			delete[] data;
			return nullptr;
		}

		// Return allocated string
		return data;
	}

	void string_table::clear_usage()
	{
		// Clear usages
		for (auto iter = _table.begin(); iter != _table.end(); ++iter)
			iter.val() = false;
	}

	void string_table::mark_used(const char* string)
	{
		auto iter = _table.find(string);
		if (iter == _table.end())
			return; // assert??

		// set used flag
		*iter = true;
	}

	void string_table::gc()
	{
		// begin at the start
		auto iter = _table.begin();

		const char* last = nullptr;
		while (iter != _table.end())
		{
			// If the string is not used
			if (!*iter)
			{
				// Delete it
				delete[] iter.key();
				_table.erase(iter);

				// Re-establish iterator at last position
				// TODO: BAD. We need inline delete that doesn't invalidate pointers
				if (last == nullptr)
					iter = _table.begin();
				else
				{
					iter = _table.find(last);
					iter++;
				}

				continue;
			}

			// Next
			last = iter.key();
			iter++;
		}
	}
}
