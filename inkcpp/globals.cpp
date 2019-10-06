#include "globals.h"
#include "story.h"

namespace ink::runtime
{
	globals::globals(const story* story)
		: _owner(story), _num_containers(story->num_containers())
		, _visit_counts(nullptr)
	{
		// Create stores
		_visit_counts = new uint32_t[_num_containers];
		inkZeroMemory(_visit_counts, _num_containers * sizeof(uint32_t));
	}

	void globals::visit(uint32_t container_id)
	{
		assert(container_id < _num_containers);
		_visit_counts[container_id]++;
	}

	uint32_t globals::visits(uint32_t container_id) const
	{
		assert(container_id < _num_containers);
		return _visit_counts[container_id];
	}
}