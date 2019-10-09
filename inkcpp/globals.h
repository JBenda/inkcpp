#pragma once

#include "system.h"
#include "array.h"

namespace ink::runtime
{
	class story;

	/**
	* Represents a global store to be shared amongst ink runners. 
	* Stores global variable values, visit counts, turn counts, etc.
	*/
	class globals
	{
	public:
		// Initializes a new global store from the given story
		globals(const story*);

	public:
		// Records a visit to a container
		void visit(uint32_t container_id);

		// Checks the number of visits to a container
		uint32_t visits(uint32_t container_id) const;

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
		const story* const _owner;
	};
}