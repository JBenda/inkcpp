#include "globals.h"
#include "story.h"

namespace ink::runtime
{
	globals::globals(const story* story)
		: _owner(story), _num_containers(story->num_containers())
		, _visit_counts(_num_containers)
	{
	}

	void globals::visit(uint32_t container_id)
	{
		_visit_counts.set(container_id, _visit_counts[container_id] + 1);
	}

	uint32_t globals::visits(uint32_t container_id) const
	{
		return _visit_counts[container_id];
	}

	void globals::save()
	{
		_visit_counts.save();
	}

	void globals::restore()
	{
		_visit_counts.restore();
	}

	void globals::forget()
	{
		_visit_counts.forget();
	}
}