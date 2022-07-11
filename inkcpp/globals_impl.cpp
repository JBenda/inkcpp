#include "globals_impl.h"
#include "story_impl.h"
#include "runner_impl.h"
#include "snapshot_impl.h"

#include <cstring>

namespace ink::runtime::internal
{
	globals_impl::globals_impl(const story_impl* story)
		: _num_containers(story->num_containers())
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

	template<value_type ty, typename T>
	optional<T> fetch_variable(const value* val) {
		if (val && val->type() == ty) {
			return optional<T>(val->get<ty>());
		}
		return {nullopt};
	}

	template<value_type ty, typename T>
	bool try_set_value(value* val, T x) {
		if (val && val->type() == ty) {
			val->set<ty>(x);
			return true;
		}
		return false;
	}

	optional<int32_t> globals_impl::get_int(hash_t name) const {
		return fetch_variable<value_type::int32,int32_t>(get_variable(name));
	}
	bool globals_impl::set_int(hash_t name, int32_t i) {
		return try_set_value<value_type::int32, int32_t>(get_variable(name), i);
	}
	optional<uint32_t> globals_impl::get_uint(hash_t name) const {
		return fetch_variable<value_type::uint32,uint32_t>(get_variable(name));
	}
	bool globals_impl::set_uint(hash_t name, uint32_t i) {
		return try_set_value<value_type::uint32, uint32_t>(get_variable(name), i);
	}
	optional<float> globals_impl::get_float(hash_t name) const {
		return fetch_variable<value_type::float32,float>(get_variable(name));
	}
	bool globals_impl::set_float(hash_t name, float i) {
		return try_set_value<value_type::float32, float>(get_variable(name), i);
	}

	optional<const char*> globals_impl::get_str(hash_t name) const {
		return fetch_variable<value_type::string, const char*>(get_variable(name));
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
		_globals_initialized = true;
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
