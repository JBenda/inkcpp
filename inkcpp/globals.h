#pragma once

#include "system.h"

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

		// Updates visit and turn counts that would result from a jump from start to end
		void handle_jump(ip_t start, ip_t end, container_t previous);

		// Checks the number of visits to a container
		uint32_t visits(uint32_t container_id) const;
	private:
		// Store the number of containers. This is the length of most of our lists
		const uint32_t _num_containers;
		
		// Visit count array
		uint32_t* _visit_counts;

		// Pointer back to owner story.
		const story* const _owner;
	};
}