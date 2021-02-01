#include "globals_impl.h"
#include "story_impl.h"
#include "runner_impl.h"

namespace ink::runtime::internal
{
	globals_impl::globals_impl(const story_impl* story)
		: _num_containers(story->num_containers())
		, _visit_counts(_num_containers, ~0)
		, _owner(story)
		, _runners_start(nullptr)
		, _globals_initialized(false)
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

	void globals_impl::add_runner(const runner_impl* runner)
	{
		// cache start of list
		auto first = _runners_start;

		// create new entry as start, linked to the previous start
		_runners_start = new runner_entry{ runner, first };
	}

	void globals_impl::remove_runner(const runner_impl* runner)
	{
		// iterate linked list
		runner_entry* prev = nullptr;
		auto iter = _runners_start;
		while (iter != nullptr)
		{
			if (iter->object == runner)
			{
				// Fixup next pointer
				if (prev != nullptr)
					prev->next = iter->next;
				else
					_runners_start = iter->next;

				// delete
				delete iter;
				return;
			}

			// move on to next entry
			prev = iter;
			iter = iter->next;
		}
	}

	void globals_impl::set_variable(hash_t name, const value& val)
	{
		_variables.set(name, val);
	}

	const value* globals_impl::get_variable(hash_t name) const
	{
		return _variables.get(name);
	}

	void globals_impl::initialize_globals(runner_impl* run)
	{
		// If no way to move there, then there are no globals.
		if (!run->move_to(hash_string("global decl")))
		{
			_globals_initialized = true;
			return;
		}

		// execute one line to startup the globals
		run->getline_silent();
	}

	void globals_impl::gc()
	{
		// Mark all strings as unused
		_strings.clear_usage();

		// Iterate runners and mark their strings
		auto iter = _runners_start;
		while (iter != nullptr)
		{
			iter->object->mark_strings(_strings);
			iter = iter->next;
		}

		// Mark our own strings
		_variables.mark_strings(_strings);

		// run garbage collection
		_strings.gc();
	}

	void globals_impl::save()
	{
		_visit_counts.save();
		_variables.save();
	}

	void globals_impl::restore()
	{
		_visit_counts.restore();
		_variables.restore();
	}

	void globals_impl::forget()
	{
		_visit_counts.forget();
		_variables.forget();
	}
}