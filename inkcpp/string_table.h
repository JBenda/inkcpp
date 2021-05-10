#pragma once

#include "avl_array.h"
#include "system.h"

namespace ink::runtime::internal
{
	// hash tree sorted by string pointers
	class string_table
	{
	public:
		~string_table();

		// Create a dynmaic string of a particular length
		char* create(size_t length);
		char* duplicate(const char* str);

		// zeroes all usage values
		void clear_usage();

		// mark a string as used
		void mark_used(const char* string);

		// deletes all unused strings
		void gc();

	private:
		avl_array<const char*, bool, ink::size_t, 100> _table;
	};
}
