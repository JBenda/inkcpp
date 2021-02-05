#pragma once

#include "system.h"
#include "array.h"
#include "globals.h"
#include "string_table.h"
#include "stack.h"

namespace ink::runtime::internal
{
	class story_impl;
	class runner_impl;

	// Implementation of the global store
	class globals_impl : public globals_interface
	{
	public:
		// Initializes a new global store from the given story
		globals_impl(const story_impl*);
		virtual ~globals_impl() { }

	protected:
		const uint32_t* getUInt(hash_t name) const override;
		uint32_t* getUInt(hash_t name) override;

	  	const int32_t* getInt(hash_t name) const override;
	  	int32_t* getInt(hash_t name) override;

		const float* getFloat(hash_t name) const override;
		float* getFloat(hash_t name) override;

		const char* getStr(hash_t name) const override;
		char* getStr(hash_t name) override;

	public:
		// Records a visit to a container
		void visit(uint32_t container_id);

		// Checks the number of visits to a container
		uint32_t visits(uint32_t container_id) const;

		// registers/unregisters a runner as using this globals object
		void add_runner(const runner_impl*);
		void remove_runner(const runner_impl*);

		// sets a global variable
		void set_variable(hash_t name, const value&);

		// gets a global variable
		const value* get_variable(hash_t name) const;
		value* get_variable(hash_t name);

		// checks if globals are initialized
		bool are_globals_initialized() const { return _globals_initialized; }

		// initializes globals using a runner
		void initialize_globals(runner_impl*);

		// gets the allocated string table
		inline string_table& strings() { return _strings; }

		// run garbage collection
		void gc();

		// == Save/Restore ==
		void save();
		void restore();
		void forget();

	private:
		// Store the number of containers. This is the length of most of our lists
		const uint32_t _num_containers;

		// Visit count array
		internal::allocated_restorable_array<uint32_t> _visit_counts;

		// Pointer back to owner story.
		const story_impl* const _owner;

		struct runner_entry
		{
			const runner_impl* object;
			runner_entry* next = nullptr;
		};

		// Linked list of runners operating under this globals object.
		//  Used for garbage collection of the global string table.
		runner_entry* _runners_start;

		// Allocated string table (shared by all runners using this global store)
		string_table _strings;

		// Global variables (TODO: Max 50?)
		//  Implemented as a stack (slow lookup) because it has save/restore functionality.
		//  If I could create an avl tree with save/restore, that'd be great but seems super complex.
		internal::stack<50> _variables;
		bool _globals_initialized;
	};
}
