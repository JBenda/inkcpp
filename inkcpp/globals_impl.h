#pragma once

#include "system.h"
#include "array.h"
#include "globals.h"
#include "string_table.h"
#include "list_table.h"
#include "stack.h"
#include "snapshot_impl.h"

namespace ink::runtime::internal
{
	class story_impl;
	class runner_impl;
	class snapshot_impl;

	// Implementation of the global store
	class globals_impl : public globals_interface, public snapshot_interface
	{
		friend snapshot_impl;
	public:
		size_t snap(unsigned char* data, const snapper&) const override;
		const unsigned char* snap_load(const unsigned char* data, const loader&) override;
		// Initializes a new global store from the given story
		globals_impl(const story_impl*);
		virtual ~globals_impl() { }

		snapshot* create_snapshot() const override;

	protected:
		optional<uint32_t> get_uint(hash_t name) const override;
		bool set_uint(hash_t name, uint32_t value) override;

	  	optional<int32_t> get_int(hash_t name) const override;
		bool set_int(hash_t name, int32_t value) override;

		optional<float> get_float(hash_t name) const override;
		bool set_float(hash_t name, float value) override;

		optional<const char*> get_str(hash_t name) const override;
		bool set_str(hash_t name, const char* value) override;

	public:
		// Records a visit to a container
		void visit(uint32_t container_id);

		// Checks the number of visits to a container
		uint32_t visits(uint32_t container_id) const;

		// Returnn number of turns since container was last visited
		// \retval -1 if container was never visited before
		uint32_t turns(uint32_t container_id) const;

		// signal that a turn is habbend (eg. a choice is taken) 
		void turn();

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
		inline const string_table& strings() const { return _strings; }

		// gets list entries
		list_table& lists() { return _lists; }

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
		struct visit_count {
			uint32_t visits = 0;
			int32_t turns = -1;
			bool operator==(const visit_count& vc) const {
				return visits == vc.visits && turns == vc.turns;
			}
			bool operator!=(const visit_count& vc) const {
				return !(*this == vc);
			}
		};
		managed_array<visit_count, true, 1> _visit_counts;

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
		mutable string_table _strings;
		mutable list_table _lists;

		//  Implemented as a stack (slow lookup) because it has save/restore functionality.
		//  If I could create an avl tree with save/restore, that'd be great but seems super complex.
		internal::stack<abs(config::limitGlobalVariables), config::limitGlobalVariables < 0> _variables;
		bool _globals_initialized;
	};
}
