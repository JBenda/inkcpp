#include "globals_impl.h"
#include "story_impl.h"
#include "runner_impl.h"
#include "snapshot_impl.h"
#include "system.h"
#include "types.h"


namespace ink::runtime::internal
{
	globals_impl::globals_impl(const story_impl* story)
		: _num_containers(story->num_containers())
		, _turn_cnt{0}
		, _visit_counts()
		, _owner(story)
		, _runners_start(nullptr)
		, _lists(story->list_meta(), story->get_header())
		, _globals_initialized(false)
	{
		_visit_counts.resize(_num_containers);
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

	void globals_impl::visit(uint32_t container_id, bool entering_at_start)
	{
		if((!(_owner->container_flag(container_id) & CommandFlag::CONTAINER_MARKER_ONLY_FIRST)) || entering_at_start) {
			_visit_counts[container_id].visits += 1;
			_visit_counts[container_id].turns  = 0;
		}
	}

	uint32_t globals_impl::visits(uint32_t container_id) const
	{
		return _visit_counts[container_id].visits;
	}

	uint32_t globals_impl::turns() const {
		return _turn_cnt;
	}

	void globals_impl::turn()
	{
		++_turn_cnt;
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
		ink::optional<value> old_var = ink::nullopt;
		value* p_old_var = get_variable(name);
		if (p_old_var != nullptr) {
			old_var = *p_old_var;
		}

		_variables.set(name, val);

		for(auto& callback : _callbacks) {
			if (callback.name == name) {
				if (old_var.has_value()) {
					callback.operation->call(val.to_interface_value(lists()), {old_var->to_interface_value(lists())});
				} else {
					callback.operation->call(val.to_interface_value(lists()), ink::nullopt);
				}
			}
		}
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
		return {var->to_interface_value(_lists)};
	}
	
	bool globals_impl::set_var(hash_t name, const ink::runtime::value& val) {
		auto* var = get_variable(name);
		if (!var) { return false; }
		ink::runtime::value old_val = var->to_interface_value(lists());

		bool ret = false;
		if ( val.type == ink::runtime::value::Type::String) {
			if (!(var->type() == value_type::none || var->type() == value_type::string)) { return false; }
			size_t size = 0;
			char* ptr;
			for ( const char* i = val.v_string; *i; ++i ) { ++size; }
			char* new_string = strings().create( size + 1 );
			strings().mark_used( new_string );
			ptr = new_string;
			for ( const char* i = val.v_string; *i; ++i ) {
				*ptr++ = *i;
			}
			*ptr = 0;
			*var = value{}.set<value_type::string>( static_cast<const char*>( new_string ), true );
			ret = true;
		} else {
			ret = var->set(val);
		}

		for (auto& callback : _callbacks) {
			if (callback.name == name) {
				callback.operation->call(val, {old_val});
			}
		}
		
		return ret;
	}

	void globals_impl::internal_observe(hash_t name, callback_base* callback) {
		_callbacks.push() = Callback { .name = name, .operation = callback };
		if (_globals_initialized) {
			value* p_var = _variables.get(name);
			inkAssert(p_var != nullptr, "Global variable to observe does not exists after initiliazation. This variable will therofe not get any value.");
			callback->call(p_var->to_interface_value(lists()), ink::nullopt);
		}
	}

	void globals_impl::initialize_globals(runner_impl* run)
	{
		// If no way to move there, then there are no globals.
		if (!run->move_to(hash_string("global decl")))
		{
			return;
		}

		// execute one line to startup the globals
		run->getline_silent();
		_globals_initialized = true;
	}

	void globals_impl::gc()
	{
		// Mark all strings as unused
		_strings.clear_usage();
		_lists.clear_usage();

		// Iterate runners and mark their strings
		auto iter = _runners_start;
		while (iter != nullptr)
		{
			iter->object->mark_used(_strings, _lists);
			iter = iter->next;
		}

		// Mark our own strings
		_variables.mark_used(_strings, _lists);

		// run garbage collection
		_strings.gc();
		_lists.gc();
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

	snapshot* globals_impl::create_snapshot() const
	{
		return new snapshot_impl(*this);
	}

	size_t globals_impl::snap(unsigned char* data, const snapper& snapper) const
	{
		unsigned char* ptr = data;
		inkAssert(_num_containers == _visit_counts.size(), "Should be equal!");
		inkAssert(_globals_initialized, "Only support snapshot of globals with runner! or you don't need a snapshot for this state");
		ptr += _visit_counts.snap( data ? ptr : nullptr, snapper );
		ptr += _strings.snap( data ? ptr : nullptr, snapper );
		ptr += _lists.snap( data ? ptr : nullptr, snapper );
		ptr += _variables.snap(data ? ptr : nullptr, snapper );
		return ptr - data;
	}

	const unsigned char* globals_impl::snap_load(const unsigned char* ptr, const loader& loader)
	{
		_globals_initialized = true;
		ptr = _visit_counts.snap_load(ptr, loader);
		inkAssert(_num_containers == _visit_counts.size(), "errer when loading visit counts, story file dont match snapshot!");
		ptr = _strings.snap_load(ptr, loader);
		ptr = _lists.snap_load(ptr, loader);
		ptr = _variables.snap_load(ptr, loader);
		return ptr;
	}
}
