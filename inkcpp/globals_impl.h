#pragma once

#include "config.h"
#include "system.h"
#include "array.h"
#include "globals.h"
#include "string_table.h"
#include "list_table.h"
#include "list_impl.h"
#include "stack.h"
#include "snapshot_impl.h"
#include "functional.h"

namespace ink::runtime::internal
{
	class story_impl;
	class runner_impl;
	class snapshot_impl;

	// Implementation of the global store
	class globals_impl final : public globals_interface, public snapshot_interface
	{
		friend snapshot_impl;
	public:
		size_t snap(unsigned char* data, const snapper&) const;
		const unsigned char* snap_load(const unsigned char* data, const loader&);
		// Initializes a new global store from the given story
		globals_impl(const story_impl*);
		virtual ~globals_impl() { }

		snapshot* create_snapshot() const override;

	protected:
		optional<ink::runtime::value> get_var(hash_t name) const override;
		bool set_var(hash_t name, const ink::runtime::value& val) override;
		void internal_observe(hash_t name, internal::callback_base* callback) override;

	public:
		// Records a visit to a container
		/// @param start_cmd iff the visit was initiatet through a MARKER_START_CONTAINER
		void visit(uint32_t container_id, bool entering_at_start);

		// Checks the number of visits to a container
		uint32_t visits(uint32_t container_id) const;
		// Number of current turn (number of passed choices)
		uint32_t turns() const;

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

		uint32_t _turn_cnt = 0;
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
		struct Callback {
			hash_t name;
			callback_base* operation;
		};
		managed_array<Callback, config::limitGlobalVariableObservers < 0, abs(config::limitGlobalVariableObservers)> _callbacks;
		bool _globals_initialized;
	};
}
