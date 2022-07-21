#include "globals_impl.h"
#include "story_impl.h"
#include "runner_impl.h"

namespace ink::runtime::internal
{
	globals_impl::globals_impl(const story_impl* story)
		: _num_containers(story->num_containers())
		, _visit_counts(_num_containers)
		, _owner(story)
		, _runners_start(nullptr)
		, _lists(story->list_meta(), story->get_header())
		, _globals_initialized(false)
	{
		if (_lists) {
			// initelize static lists
			const list_flag* flags = story->lists();
			while(*flags != null_flag) {
				list_table::list l = _lists.create_permament();
				while(*flags != null_flag) {
					_lists.add_inplace(l, *flags);
					++flags;
				}
				++flags;
			}
			for(const auto& flag : _lists.named_flags()) {
				set_variable(hash_string(flag.name), value{}.set<value_type::list_flag>(
						list_flag{
							flag.flag.list_id,
							flag.flag.flag
						}));
			}
		}
	}

	void globals_impl::visit(uint32_t container_id)
	{
		_visit_counts[container_id].visits += 1;
		_visit_counts[container_id].turns  = 0;
	}

	uint32_t globals_impl::visits(uint32_t container_id) const
	{
		return _visit_counts[container_id].visits;
	}

	void globals_impl::turn()
	{
		for(size_t i = 0; i < _visit_counts.size(); ++i)
		{
			if(_visit_counts[i].turns != -1) {
				_visit_counts[i].turns += 1;
			}
		}
	}

	uint32_t globals_impl::turns(uint32_t container_id) const
	{
		return _visit_counts[container_id].turns;
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

	value* globals_impl::get_variable(hash_t name) {
		return _variables.get(name);
	}

	optional<ink::runtime::value> globals_impl::get_var(hash_t name) const {
		auto* var = get_variable(name);
		if (!var) { return nullopt; }
		return {var.to_interface_value()};
	}
	
	bool globals_impl::set_var(hash_t name, const ink::runtime::value& val) {
		auto* var = get_variable(name);
		if (!var) { return false; }
		var = val;
		return true;
	}
	
	bool globals_impl::set_str(hash_t name, const char* val) {
		value* v = get_variable(name);
		if (v->type() == value_type::string)
		{
			size_t size = 0;
			char* ptr;
			for(const char*i = val; *i; ++i) { ++size; }
			char* new_string = strings().create(size + 1);
			strings().mark_used(new_string);
			ptr = new_string;
			for(const char* i = val; *i; ++i) {
				*ptr++ = *i;
			}
			*ptr = 0;
			*v = value{}.set<value_type::string>(static_cast<const char*>(new_string), true);
			return true;
		}
		return false;
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
		_variables.save();
	}

	void globals_impl::restore()
	{
		_variables.restore();
	}

	void globals_impl::forget()
	{
		_variables.forget();
	}
}
