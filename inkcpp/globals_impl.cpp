#include "globals_impl.h"
#include "story_impl.h"

namespace ink::runtime::internal
{
	globals_impl::globals_impl(const story_impl* story)
		: _owner(story), _num_containers(story->num_containers())
		, _visit_counts(_num_containers)
	{
	}

	void globals_impl::visit(uint32_t container_id)
	{
		_visit_counts.set(container_id, _visit_counts[container_id] + 1);
	}

	uint32_t globals_impl::visits(uint32_t container_id) const
	{
		return _visit_counts[container_id];
	}

	void globals_impl::save()
	{
		_visit_counts.save();
	}

	void globals_impl::restore()
	{
		_visit_counts.restore();
	}

	void globals_impl::forget()
	{
		_visit_counts.forget();
	}
}