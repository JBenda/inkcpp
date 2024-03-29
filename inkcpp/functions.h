/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
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
	function_base* find(hash_t name);

private:
	struct entry {
		hash_t         name;
		function_base* value;
		entry*         next;
	};

	// TODO: Better than a linked list?
	entry* _list;
	entry* _last;
};
} // namespace ink::runtime::internal
