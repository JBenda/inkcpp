#pragma once

#include "functional.h"
#include "system.h"

namespace ink::runtime::internal
{
	class basic_eval_stack;

	// Stores bound functions
	class functions
	{
	public:
		functions();
		~functions();

		// Adds a function to the registry
		void add(hash_t name, function_base* func);

		// Calls a function (if available)
		bool call(hash_t name, basic_eval_stack* stack, size_t num_arguments, string_table& strings, list_table& lists);

	private:
		struct entry
		{
			hash_t name;
			function_base* value;
			entry* next;
		};

		// TODO: Better than a linked list?
		entry* _list;
		entry* _last;
	};
}