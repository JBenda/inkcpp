#pragma once

#include "system.h"
#include "array.h"
#include "globals.h"

namespace ink::runtime::internal
{
	class story_impl;

	// Implementation of the global store
	class globals_impl : public globals_interface
	{
	public:
		// Initializes a new global store from the given story
		globals_impl(const story_impl*);

		virtual void dummy() override { }

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
		const story_impl* const _owner;
	};
}