#pragma once

#include "avl_array.h"
#include "system.h"
#include "snapshot_impl.h"

namespace ink::runtime::internal
{
	// hash tree sorted by string pointers
	class string_table : public snapshot_interface
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


		// snapshot interface implementation
		size_t snap(unsigned char* data, const snapper&) const override;
		const unsigned char* snap_load(const unsigned char* data, const loader&) override;

		// get position of string when iterate through data
		// used to enable storing a string table references
		size_t get_id(const char* string) const;

		// deletes all unused strings
		void gc();

	private:
		avl_array<const char*, bool, ink::size_t, 100> _table;
	};
}
