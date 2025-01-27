/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "runner_impl.h"

#include <algorithm>
#include <iomanip>

#include "choice.h"
#include "command.h"
#include "globals_impl.h"
#include "header.h"
#include "snapshot_impl.h"
#include "story_impl.h"
#include "system.h"
#include "value.h"

namespace ink::runtime
{
static void write_hash(std::ostream& out, ink::hash_t value)
{
	using namespace std;

	ios::fmtflags state{out.flags()};
	out << "0x" << hex << setfill('0') << setw(8) << static_cast<uint32_t>(value);
	out.flags(state);
}

const choice* runner_interface::get_choice(size_t index) const
{
	inkAssert(index < num_choices(), "Choice out of bounds!");
	return begin() + index;
}

size_t runner_interface::num_choices() const { return end() - begin(); }
} // namespace ink::runtime

namespace ink::runtime::internal
{

template<>
value* runner_impl::get_var<runner_impl::Scope::GLOBAL>(hash_t variableName)
{
	return _globals->get_variable(variableName);
}

template<>
value* runner_impl::get_var<runner_impl::Scope::LOCAL>(hash_t variableName)
{
	value* ret = _stack.get(variableName);
	if (! ret) {
		return nullptr;
	}
	if (ret->type() == value_type::value_pointer) {
		auto [name, ci] = ret->get<value_type::value_pointer>();
		inkAssert(ci == 0, "only Global pointer are allowd on the _stack!");
		return get_var<runner_impl::Scope::GLOBAL>(name);
	}
	return ret;
}

template<>
value* runner_impl::get_var<runner_impl::Scope::NONE>(hash_t variableName)
{
	value* ret = get_var<Scope::LOCAL>(variableName);
	if (ret) {
		return ret;
	}
	return get_var<Scope::GLOBAL>(variableName);
}

template<runner_impl::Scope Hint>
const value* runner_impl::get_var(hash_t variableName) const
{
	return const_cast<runner_impl*>(this)->get_var<Hint>(variableName);
}

template<>
void runner_impl::set_var<runner_impl::Scope::GLOBAL>(
    hash_t variableName, const value& val, bool is_redef
)
{
	if (is_redef) {
		value* src = _globals->get_variable(variableName);
		_globals->set_variable(variableName, src->redefine(val, _globals->lists()));
	} else {
		_globals->set_variable(variableName, val);
	}
}

const value* runner_impl::dereference(const value& val)
{
	if (val.type() != value_type::value_pointer) {
		return &val;
	}

	auto [name, ci] = val.get<value_type::value_pointer>();
	if (ci == 0) {
		return get_var<Scope::GLOBAL>(name);
	}
	return _stack.get_from_frame(ci, name);
}

template<>
void runner_impl::set_var<runner_impl::Scope::LOCAL>(
    hash_t variableName, const value& val, bool is_redef
)
{
	if (val.type() == value_type::value_pointer) {
		inkAssert(is_redef == false, "value pointer can only use to initelize variable!");
		auto [name, ci] = val.get<value_type::value_pointer>();
		if (ci == 0) {
			_stack.set(variableName, val);
		} else {
			const value* dref = dereference(val);
			if (dref == nullptr) {
				value v   = val;
				auto  ref = v.get<value_type::value_pointer>();
				v.set<value_type::value_pointer>(ref.name, 0);
				_stack.set(variableName, v);
			} else {
				_ref_stack.set(variableName, val);
				_stack.set(variableName, *dref);
			}
		}
	} else {
		if (is_redef) {
			value* src = _stack.get(variableName);
			if (src->type() == value_type::value_pointer) {
				auto [name, ci] = src->get<value_type::value_pointer>();
				inkAssert(ci == 0, "Only global pointer are allowed on _stack!");
				set_var<Scope::GLOBAL>(
				    name, get_var<Scope::GLOBAL>(name)->redefine(val, _globals->lists()), true
				);
			} else {
				_stack.set(variableName, src->redefine(val, _globals->lists()));
			}
		} else {
			_stack.set(variableName, val);
		}
	}
}

template<>
void runner_impl::set_var<runner_impl::Scope::NONE>(
    hash_t variableName, const value& val, bool is_redef
)
{
	inkAssert(is_redef, "define set scopeless variables!");
	if (_stack.get(variableName)) {
		return set_var<Scope::LOCAL>(variableName, val, is_redef);
	} else {
		return set_var<Scope::GLOBAL>(variableName, val, is_redef);
	}
}

template<typename T>
inline T runner_impl::read()
{
	using header = ink::internal::header;
	// Sanity
	inkAssert(_ptr + sizeof(T) <= _story->end(), "Unexpected EOF in Ink execution");

	// Read memory
	T val = *( const T* ) _ptr;
	if (_story->get_header().endien == header::endian_types::differ) {
		val = header::swap_bytes(val);
	}

	// Advance ip
	_ptr += sizeof(T);

	// Return
	return val;
}

template<>
inline const char* runner_impl::read()
{
	offset_t str = read<offset_t>();
	return _story->string(str);
}

choice& runner_impl::add_choice()
{
	inkAssert(
	    config::maxChoices < 0 || _choices.size() < static_cast<size_t>(abs(config::maxChoices)),
	    "Ran out of choice storage!"
	);
	return _choices.push();
}

void runner_impl::clear_choices()
{
	// TODO: Garbage collection? ? which garbage ?
	_fallback_choice = nullopt;
	_choices.clear();
}

snap_tag& runner_impl::add_tag(const char* value, tags_level where)
{
	// Determine insertion index for tag level
	size_t index = 0;
	switch (where) {
		case tags_level::GLOBAL:
			index = _global_tags_count;
			_global_tags_count++;
			break;
		case tags_level::CHOICE:
			index = _global_tags_count + _choice_tags_count;
			_choice_tags_count++;
			break;
		case tags_level::LINE: index = _tags.size(); break;
		default:
			inkAssert(false, "Failed to add tag at unhandled location %d.", static_cast<int32_t>(where));
			break;
	}

	return _tags.insert(index) = value;
}

void runner_impl::clear_tags(tags_clear_type type /*= tags_clear_type::KEEP_GLOBALS*/)
{
	// To explain how this works, let's imagine we have the following tags:
	//
	// index | length | tag value      | level
	// ------|--------|----------------|---------
	// 0     | 1      | global_tag     | GLOBAL
	// 1     | 2      | choice_tag_1   | CHOICE
	// 2     | 3      | choice_tag_2   | CHOICE
	// 3     | 4      | content_tag_1  | LINE
	//
	// * Clearing all tags means we clear the entire array and clear the counts
	// * Keeping the global tags means resizing to length 1 and clearing the tags count
	// * Keeping the choice tags means resizing to length 3 (globals + choices)

	switch (type) {
		case tags_clear_type::ALL:
			_tags.clear();
			_global_tags_count = 0;
			_choice_tags_begin = ~0;
			_choice_tags_count = 0;
			break;
		case tags_clear_type::KEEP_GLOBALS:
			_tags.resize(_global_tags_count);
			_choice_tags_begin = ~0;
			_choice_tags_count = 0;
			break;
		case tags_clear_type::KEEP_CHOICE: _tags.resize(_global_tags_count + _choice_tags_count); break;
		default:
			inkAssert(false, "Unhandled clear type %d for tags.", static_cast<int32_t>(type));
			break;
	}
}

void runner_impl::jump(ip_t dest, bool record_visits)
{
	// Optimization: if we are _is_falling, then we can
	//  _should be_ able to safely assume that there is nothing to do here. A falling
	//  divert should only be taking us from a container to that same container's end point
	//  without entering any other containers
	// OR IF if target is same position do nothing
	// could happend if jumping to and of an unnamed container
	if (dest == _ptr) {
		_ptr = dest;
		return;
	}

	const uint32_t* iter = nullptr;
	container_t     id;
	ip_t            offset = nullptr;
	size_t          comm_end;
	bool            reversed = _ptr > dest;

	if (reversed) {
		comm_end                            = 0;
		iter                                = nullptr;
		const ContainerData* old_iter       = nullptr;
		const uint32_t*      last_comm_iter = nullptr;
		_container.rev_iter(old_iter);

		// find commen part of old and new stack
		while (_story->iterate_containers(iter, id, offset)) {
			if (old_iter == nullptr || offset >= dest) {
				break;
			}
			if (old_iter != nullptr && id == old_iter->id) {
				last_comm_iter = iter;
				_container.rev_iter(old_iter);
				++comm_end;
			}
		}

		// clear old part from stack
		while (_container.size() > comm_end) {
			_container.pop();
		}
		iter = last_comm_iter;

	} else {
		iter     = nullptr;
		comm_end = _container.size();
		// go to current possition in container list
		while (_story->iterate_containers(iter, id, offset)) {
			if (offset >= _ptr) {
				break;
			}
		}
		_story->iterate_containers(iter, id, offset, true);
	}

	// move to destination and update container stack on the go
	while (_story->iterate_containers(iter, id, offset)) {
		if (offset >= dest) {
			break;
		}
		if (_container.empty() || _container.top().id != id) {
			_container.push({.id = id, .offset = offset});
		} else {
			_container.pop();
			if (_container.size() < comm_end) {
				comm_end = _container.size();
			}
		}
	}
	_ptr = dest;

	// if we jump directly to a named container start, go inside, if its a ONLY_FIRST container
	// it will get visited in the next step
	if (offset == dest && static_cast<Command>(offset[0]) == Command::START_CONTAINER_MARKER) {
		_ptr += 6;
		_container.push({.id = id, .offset = offset});
		if (reversed && comm_end == _container.size() - 1) {
			++comm_end;
		}
	}

	// iff all container (until now) are entered at first position
	bool allEnteredAtStart = true;
	ip_t child_position    = dest;
	if (record_visits) {
		const ContainerData* iData = nullptr;
		size_t               level = _container.size();
		while (_container.iter(iData)
		       && (level > comm_end
		           || _story->container_flag(iData->offset) & CommandFlag::CONTAINER_MARKER_ONLY_FIRST)
		) {
			auto parrent_offset = iData->offset;
			inkAssert(child_position >= parrent_offset, "Container stack order is broken");
			// 6 == len of START_CONTAINER_SIGNAL, if its 6 bytes behind the container it is a unnnamed
			// subcontainers first child check if child_positino is the first child of current container
			allEnteredAtStart = allEnteredAtStart && ((child_position - parrent_offset) <= 6);
			child_position    = parrent_offset;
			_globals->visit(iData->id, allEnteredAtStart);
		}
	}
}

template<frame_type type>
void runner_impl::start_frame(uint32_t target)
{
	if constexpr (type == frame_type::function) {
		// add a function start marker
		_output << values::func_start;
	}
	// Push next address onto the callstack
	{
		size_t address = _ptr - _story->instructions();
		_stack.push_frame<type>(address, _evaluation_mode);
		_ref_stack.push_frame<type>(address, _evaluation_mode);
	}
	_evaluation_mode = false; // unset eval mode when enter function or tunnel

	// Do the jump
	inkAssert(_story->instructions() + target < _story->end(), "Diverting past end of story data!");
	jump(_story->instructions() + target, true);
}

frame_type runner_impl::execute_return()
{
	// Pop the callstack
	_ref_stack.fetch_values(_stack);
	frame_type type;
	offset_t   offset = _stack.pop_frame(&type, _evaluation_mode);
	_ref_stack.push_values(_stack);
	{
		frame_type t;
		bool       eval;
		// TODO: write all refs to new frame
		offset_t   o = _ref_stack.pop_frame(&t, eval);
		inkAssert(
		    o == offset && t == type && eval == _evaluation_mode,
		    "_ref_stack and _stack should be in frame sync!"
		);
	}

	// SPECIAL: On function, do a trim
	if (type == frame_type::function) {
		_output << values::func_end;
	} else if (type == frame_type::tunnel) {
		// if we return but there is a divert target on top of
		// the evaluation stack, we should follow this instead
		// inkproof: I060
		if (! _eval.is_empty() && _eval.top().type() == value_type::divert) {
			start_frame<frame_type::tunnel>(_eval.pop().get<value_type::divert>());
			return type;
		}
	}


	// Jump to the old offset
	inkAssert(
	    _story->instructions() + offset < _story->end(),
	    "Callstack return is outside bounds of story!"
	);
	jump(_story->instructions() + offset, false);

	// Return frame type
	return type;
}

runner_impl::runner_impl(const story_impl* data, globals global)
    : _story(data)
    , _globals(global.cast<globals_impl>())
    , _operations(
          global.cast<globals_impl>()->strings(), global.cast<globals_impl>()->lists(), _rng,
          *global.cast<globals_impl>(), *data, static_cast<const runner_interface&>(*this)
      )
    , _ptr(_story->instructions())
    , _container(ContainerData{})
    , _rng(time(nullptr))
{
	// register with globals
	_globals->add_runner(this);
	if (_globals->lists()) {
		_output.set_list_meta(_globals->lists());
	}

	// initialize globals if necessary
	if (! _globals->are_globals_initialized()) {
		_globals->initialize_globals(this);

		// Set us back to the beginning of the story
		reset();
		_ptr = _story->instructions();
	}
}

runner_impl::~runner_impl()
{
	// unregister with globals
	if (_globals.is_valid()) {
		_globals->remove_runner(this);
	}
}

runner_impl::line_type runner_impl::getline()
{
	// Advance interpreter one line and write to output
	advance_line(_debug_stream);

#ifdef INK_ENABLE_STL
	line_type result{_output.get()};
#elif defined(INK_ENABLE_UNREAL)
	line_type result{ANSI_TO_TCHAR(_output.get_alloc(_globals->strings(), _globals->lists()))};
#endif

	// Fall through the fallback choice, if available
	if (! has_choices() && _fallback_choice) {
		choose(~0);
	}
	inkAssert(_output.is_empty(), "Output should be empty after getline!");

	return result;
}

runner_impl::line_type runner_impl::getall()
{
	if (_debug_stream != nullptr) {
		_debug_stream->clear();
	}

	line_type result;

	while (can_continue()) {
		result += getline();
	}
	inkAssert(_output.is_empty(), "Output should be empty after getall!");

	return result;
}

#ifdef INK_ENABLE_STL
void runner_impl::getline(std::ostream& out) { out << getline(); }

void runner_impl::getall(std::ostream& out)
{
	// Advance interpreter and write lines to output
	while (can_continue()) {
		out << getline();
	}
	inkAssert(_output.is_empty(), "Output should be empty after getall!");
}
#endif

void runner_impl::advance_line(std::ostream* debug_stream /*= nullptr*/)
{
	clear_tags(tags_clear_type::KEEP_GLOBALS);

	// Step while we still have instructions to execute
	while (_ptr != nullptr) {
		// Stop if we hit a new line
		if (line_step(debug_stream)) {
			break;
		}
	}

	// can be in save state becaues of choice
	// Garbage collection TODO: How often do we want to do this?
	if (_saved) {
		restore();
	}
	_globals->gc();
	if (_output.saved()) {
		_output.restore();
	}
}

bool runner_impl::can_continue() const { return _ptr != nullptr && ! has_choices(); }

void runner_impl::choose(size_t index)
{
	if (has_choices()) {
		inkAssert(index < _choices.size(), "Choice index out of range");
	}
	_globals->turn();
	// Get the choice
	const auto& c = has_choices() ? _choices[index] : _fallback_choice.value();

	// Get its thread
	thread_t choiceThread = c._thread;

	// Figure out where our previous pointer was for that thread
	ip_t prev = nullptr;
	if (choiceThread == ~0) {
		prev = _done;
	} else {
		prev = _threads.get(choiceThread);
	}

	// Make sure we have a previous pointer
	inkAssert(prev != nullptr, "No 'done' point recorded before finishing choice output");

	// Move to the previous pointer so we track our movements correctly
	jump(prev, false);
	_done = nullptr;

	// Collapse callstacks to the correct thread
	_stack.collapse_to_thread(choiceThread);
	_ref_stack.collapse_to_thread(choiceThread);
	_threads.clear();

	// Jump to destination and clear choice list
	jump(_story->instructions() + c.path(), true);
	clear_choices();
	clear_tags();
}

void runner_impl::getline_silent()
{
	// advance and clear output stream
	advance_line();
	_output.clear();
}

const char* runner_impl::get_tag(size_t index) const
{
	const size_t adjusted = index + _global_tags_count;
	return (adjusted < _tags.size()) ? _tags[adjusted] : nullptr;
}

const char* runner_impl::get_global_tag(size_t index) const
{
	return (index < _global_tags_count) ? _tags[index] : nullptr;
}

snapshot* runner_impl::create_snapshot() const { return _globals->create_snapshot(); }

size_t runner_impl::snap(unsigned char* data, snapper& snapper) const
{
	unsigned char* ptr          = data;
	bool           should_write = data != nullptr;
	snapper.runner_tags         = _tags.begin();
	std::uintptr_t offset       = _ptr != nullptr ? _ptr - _story->instructions() : 0;
	ptr                         = snap_write(ptr, offset, should_write);
	offset                      = _backup - _story->instructions();
	ptr                         = snap_write(ptr, offset, should_write);
	offset                      = _done - _story->instructions();
	ptr                         = snap_write(ptr, offset, should_write);
	ptr                         = snap_write(ptr, _rng.get_state(), should_write);
	ptr                         = snap_write(ptr, _evaluation_mode, should_write);
	ptr                         = snap_write(ptr, _string_mode, should_write);
	ptr                         = snap_write(ptr, _tag_mode, should_write);
	ptr                         = snap_write(ptr, _saved_evaluation_mode, should_write);
	ptr                         = snap_write(ptr, _saved, should_write);
	ptr                         = snap_write(ptr, _is_falling, should_write);
	ptr += _output.snap(data ? ptr : nullptr, snapper);
	ptr += _stack.snap(data ? ptr : nullptr, snapper);
	ptr += _ref_stack.snap(data ? ptr : nullptr, snapper);
	ptr += _eval.snap(data ? ptr : nullptr, snapper);
	ptr = snap_write(ptr, _tags_where, should_write);
	ptr = snap_write(ptr, _choice_tags_begin, should_write);
	ptr = snap_write(ptr, _global_tags_count, should_write);
	ptr = snap_write(ptr, _choice_tags_count, should_write);
	ptr += _tags.snap(data ? ptr : nullptr, snapper);
	ptr += _container.snap(data ? ptr : nullptr, snapper);
	ptr += _threads.snap(data ? ptr : nullptr, snapper);
	ptr = snap_write(ptr, _fallback_choice.has_value(), should_write);
	if (_fallback_choice) {
		ptr += _fallback_choice.value().snap(data ? ptr : nullptr, snapper);
	}
	ptr += _choices.snap(data ? ptr : nullptr, snapper);
	return ptr - data;
}

const unsigned char* runner_impl::snap_load(const unsigned char* data, loader& loader)
{
	auto           ptr = data;
	std::uintptr_t offset;
	ptr     = snap_read(ptr, offset);
	_ptr    = offset == 0 ? nullptr : _story->instructions() + offset;
	ptr     = snap_read(ptr, offset);
	_backup = _story->instructions() + offset;
	ptr     = snap_read(ptr, offset);
	_done   = _story->instructions() + offset;
	int32_t seed;
	ptr = snap_read(ptr, seed);
	_rng.srand(seed);
	ptr                = snap_read(ptr, _evaluation_mode);
	ptr                = snap_read(ptr, _string_mode);
	ptr                = snap_read(ptr, _tag_mode);
	ptr                = snap_read(ptr, _saved_evaluation_mode);
	ptr                = snap_read(ptr, _saved);
	ptr                = snap_read(ptr, _is_falling);
	ptr                = _output.snap_load(ptr, loader);
	ptr                = _stack.snap_load(ptr, loader);
	ptr                = _ref_stack.snap_load(ptr, loader);
	ptr                = _eval.snap_load(ptr, loader);
	ptr                = snap_read(ptr, _tags_where);
	ptr                = snap_read(ptr, _choice_tags_begin);
	ptr                = snap_read(ptr, _global_tags_count);
	ptr                = snap_read(ptr, _choice_tags_count);
	ptr                = _tags.snap_load(ptr, loader);
	loader.runner_tags = _tags.begin();
	ptr                = _container.snap_load(ptr, loader);
	ptr                = _threads.snap_load(ptr, loader);
	bool has_fallback_choice;
	ptr              = snap_read(ptr, has_fallback_choice);
	_fallback_choice = nullopt;
	if (has_fallback_choice) {
		_fallback_choice.emplace();
		ptr = _fallback_choice.value().snap_load(ptr, loader);
	}
	ptr = _choices.snap_load(ptr, loader);
	return ptr;
}

#ifdef INK_ENABLE_CSTD
const char* runner_impl::getline_alloc()
{
	advance_line(_debug_stream);
	const char* res = _output.get_alloc(_globals->strings(), _globals->lists());
	if (! has_choices() && _fallback_choice) {
		choose(~0);
	}
	inkAssert(_output.is_empty(), "Output should be empty after getline!");
	return res;
}
#endif

bool runner_impl::move_to(hash_t path)
{
	// find the path
	ip_t destination = _story->find_offset_for(path);
	if (destination == nullptr) {
		// TODO: Error state?
		return false;
	}

	// Clear state and move to destination
	reset();
	_ptr = _story->instructions();
	jump(destination, false);

	return true;
}

void runner_impl::internal_bind(hash_t name, internal::function_base* function)
{
	_functions.add(name, function);
}

runner_impl::change_type runner_impl::detect_change() const
{
	inkAssert(_output.saved(), "Cannot detect changes in non-saved stream.");

	// Check if the old newline is still present (hasn't been glued)
	const bool stillHasNewline = _output.ends_with(value_type::newline, _output.save_offset());
	if (! stillHasNewline) {
		return change_type::newline_removed;
	}

	return change_type::extended_past_newline;
}

bool runner_impl::line_step(std::ostream* debug_stream /*= nullptr*/)
{
	// Track if added tags are global ones
	const bool at_story_start = _ptr == _story->instructions();

	// Step the interpreter until we've parsed all tags for the line
	size_t last_newline = basic_stream::npos;
	while (_ptr != nullptr && last_newline == basic_stream::npos) {
		step(debug_stream);

		last_newline = _output.find_last_of(value_type::newline);
	}

	// Copy global tags to the first line
	if (at_story_start && _global_tags_count > 0) {
		for (size_t i = 0; i < _global_tags_count; ++i) {
			add_tag(get_global_tag(i), tags_level::LINE);
		}
	}

	// Next tags are always added to the line
	_tags_where = tags_level::LINE;

	// Unless we are out of content, we are going to try
	//  to continue a little further. This is to check for
	//  glue (which means there is potentially more content
	//  in this line) OR for non-text content such as choices.
	if (_ptr != nullptr) {
		// Save a snapshot
		if (_saved) {
			forget();
		}
		save();

		// Step execution until we're satisfied it doesn't affect the current line
		bool keep_stepping = false;
		bool is_gluing     = false;
		bool is_newline    = false;
		do {
			if (_ptr == nullptr) {
				break;
			}

			step(debug_stream);

			// If we find glue, keep going until the next line
			if (is_gluing) {
				is_gluing = ! _output.text_past_save();
			} else {
				is_gluing = _output.ends_with(value_type::glue);

				// Make a new save point to track the glue changes
				if (is_gluing) {
					is_newline = true;

					if (_saved) {
						forget();
					}
					save();
				}
			}

			// Are we gluing?
			keep_stepping = is_gluing;

			// Find the next newline after gluing
			if (! keep_stepping && is_newline) {
				keep_stepping |= ! _output.ends_with(value_type::newline);

				// Just one more command...
				/*if (!keep_stepping) {
				  keep_stepping = true;
				  is_newline = false;
				}*/
			}

			// Are we diverting?
			keep_stepping |= _evaluation_mode;

			// Are we inside a function?
			keep_stepping = keep_stepping || _output.ends_with(value_type::func_start);
			keep_stepping = keep_stepping || _output.ends_with(value_type::func_end);

			// Haven't added more text
			keep_stepping = keep_stepping || ! _output.text_past_save();

		} while (keep_stepping);

		// If we have a saved state after a previous newline
		// don't do this if we behind choice
		if (! has_choices() && ! _fallback_choice) {
			// Check for changes in the output stream
			switch (detect_change()) {
				case change_type::extended_past_newline:
					// We've gone too far. Restore to before we moved past the newline and return
					// that we are done
					restore();
					return true;
				case change_type::newline_removed:
					// Newline was removed. Proceed as if we never hit it
					forget();
					return false;
				case change_type::no_change: break;
			}
		}

		return false;
	}

	// Otherwise, make sure we don't have any snapshots hanging around
	// expect we are in choice handling
	if (! has_choices() && ! _fallback_choice) {
		forget();
	} else {
		_output.forget();
	}

	return true;
}

void runner_impl::step(std::ostream* debug_stream /*= nullptr*/)
{
#ifndef INK_ENABLE_UNREAL
	try
#endif
	{
		inkAssert(_ptr != nullptr, "Can not step! Do not have a valid pointer");

		// Load current command
		Command     cmd  = read<Command>();
		CommandFlag flag = read<CommandFlag>();

		if (debug_stream != nullptr) {
			*debug_stream << "cmd " << cmd << " flags " << flag << " ";
		}

		// If we're falling and we hit a non-fallthrough command, stop the fall.
		if (_is_falling
		    && ! (
		        (cmd == Command::DIVERT && flag & CommandFlag::DIVERT_IS_FALLTHROUGH)
		        || cmd == Command::END_CONTAINER_MARKER
		    )) {
			_is_falling = false;
			set_done_ptr(nullptr);
		}
		if (cmd >= Command::OP_BEGIN && cmd < Command::OP_END) {
			_operations(cmd, _eval);
		} else {
			switch (cmd) {
					// == Value Commands ==
				case Command::STR: {
					const char* str = read<const char*>();

					if (debug_stream != nullptr) {
						*debug_stream << "str \"" << str << "\"";
					}

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::string>(str));
					} else {
						_output << value{}.set<value_type::string>(str);
					}
				} break;
				case Command::INT: {
					int val = read<int>();

					if (debug_stream != nullptr) {
						*debug_stream << "int " << val;
					}

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::int32>(val));
					}
					// TEST-CASE B006 don't print integers
				} break;
				case Command::BOOL: {
					bool val = read<int>() ? true : false;

					if (debug_stream != nullptr) {
						*debug_stream << "bool " << (val ? "true" : "false");
					}

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::boolean>(val));
					} else {
						_output << value{}.set<value_type::boolean>(val);
					}
				} break;
				case Command::FLOAT: {
					float val = read<float>();

					if (debug_stream != nullptr) {
						*debug_stream << "float " << val;
					}

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::float32>(val));
					}
					// TEST-CASE B006 don't print floats
				} break;
				case Command::VALUE_POINTER: {
					hash_t val = read<hash_t>();

					if (debug_stream != nullptr) {
						*debug_stream << "value_pointer ";
						write_hash(*debug_stream, val);
					}

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::value_pointer>(val, static_cast<char>(flag) - 1));
					} else {
						inkFail("never conciderd what should happend here! (value pointer print)");
					}
				} break;
				case Command::LIST: {
					list_table::list list(read<int>());

					if (debug_stream != nullptr) {
						*debug_stream << "list " << list.lid;
					}

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::list>(list));
					} else {
						char* str = _globals->strings().create(_globals->lists().stringLen(list) + 1);
						_globals->lists().toString(str, list)[0] = 0;
						_output << value{}.set<value_type::string>(str);
					}
				} break;
				case Command::DIVERT_VAL: {
					inkAssert(_evaluation_mode, "Can not push divert value into the output stream!");

					// Push the divert target onto the stack
					uint32_t target = read<uint32_t>();
					_eval.push(value{}.set<value_type::divert>(target));
				} break;
				case Command::NEWLINE: {
					if (_evaluation_mode) {
						_eval.push(values::newline);
					} else {
						if (! _output.ends_with(value_type::newline)) {
							_output << values::newline;
						}
					}
				} break;
				case Command::GLUE: {
					if (_evaluation_mode) {
						_eval.push(values::glue);
					} else {
						_output << values::glue;
					}
				} break;
				case Command::VOID: {
					if (_evaluation_mode) {
						_eval.push(values::null); // TODO: void type?
					}
				} break;

				// == Divert commands
				case Command::DIVERT: {
					// Find divert address
					uint32_t target = read<uint32_t>();

					if (debug_stream != nullptr) {
						*debug_stream << "target " << target;
					}

					// Check for condition
					if (flag & CommandFlag::DIVERT_HAS_CONDITION && ! _eval.pop().truthy(_globals->lists())) {
						break;
					}

					// SPECIAL: Fallthrough divert. We're starting to fall out of containers
					if (flag & CommandFlag::DIVERT_IS_FALLTHROUGH && ! _is_falling) {
						// Record the position of the instruction pointer at the first fallthrough.
						//  We'll use this if we run out of content and hit an implied "done" to restore
						//  our position when a choice is chosen. See ::choose
						set_done_ptr(_ptr);
						_is_falling = true;
					}

					// If we're falling out of the story, then we're hitting an implied done
					if (_is_falling && _story->instructions() + target == _story->end()) {
						// Wait! We may be returning from a function!
						frame_type type;
						if (_stack.has_frame(&type)
						    && type == frame_type::function) // implicit return is only for functions
						{
							// push null and return
							_eval.push(values::null);

							// HACK
							_ptr += sizeof(Command) + sizeof(CommandFlag);
							execute_return();
						} else {
							on_done(false);
						}
						break;
					}

					// Do the jump
					inkAssert(
					    _story->instructions() + target < _story->end(), "Diverting past end of story data!"
					);
					jump(_story->instructions() + target, true);
				} break;
				case Command::DIVERT_TO_VARIABLE: {
					// Get variable value
					hash_t variable = read<hash_t>();

					if (debug_stream != nullptr) {
						*debug_stream << "variable ";
						write_hash(*debug_stream, variable);
					}

					// Check for condition
					if (flag & CommandFlag::DIVERT_HAS_CONDITION && ! _eval.pop().truthy(_globals->lists())) {
						break;
					}

					const value* val = get_var(variable);
					inkAssert(val, "Jump destiniation needs to be defined!");

					// Move to location
					jump(_story->instructions() + val->get<value_type::divert>(), true);
					inkAssert(_ptr < _story->end(), "Diverted past end of story data!");
				} break;

				// == Terminal commands
				case Command::DONE: on_done(true); break;

				case Command::END: _ptr = nullptr; break;

				// == Tunneling
				case Command::TUNNEL: {
					uint32_t target;
					// Find divert address
					if (flag & CommandFlag::TUNNEL_TO_VARIABLE) {
						hash_t       var_name = read<hash_t>();
						const value* val      = get_var(var_name);
						inkAssert(val != nullptr, "Variable containing tunnel target could not be found!");
						target = val->get<value_type::divert>();
					} else {
						target = read<uint32_t>();
					}

					if (debug_stream != nullptr) {
						*debug_stream << "target " << target;
					}

					start_frame<frame_type::tunnel>(target);
				} break;
				case Command::FUNCTION: {
					uint32_t target;
					// Find divert address
					if (flag & CommandFlag::FUNCTION_TO_VARIABLE) {
						hash_t       var_name = read<hash_t>();
						const value* val      = get_var(var_name);
						inkAssert(val != nullptr, "Varibale containing function could not be found!");
						target = val->get<value_type::divert>();
					} else {
						target = read<uint32_t>();
					}

					if (debug_stream != nullptr) {
						*debug_stream << "target " << target;
					}

					if (! (flag & CommandFlag::FALLBACK_FUNCTION)) {
						start_frame<frame_type::function>(target);
					} else {
						inkAssert(! _eval.is_empty(), "fallback function but no function call before?");
						if (_eval.top_value().type() == value_type::ex_fn_not_found) {
							_eval.pop();
							inkAssert(
							    target != 0,
							    "Exetrnal function was not binded, and no fallback function provided!"
							);
							start_frame<frame_type::function>(target);
						}
					}
				} break;
				case Command::TUNNEL_RETURN:
				case Command::FUNCTION_RETURN: {
					execute_return();
				} break;

				case Command::THREAD: {
					// Push a thread frame so we can return easily
					// TODO We push ahead of a single divert. Is that correct in all cases....?????
					auto returnTo = _ptr + CommandSize<uint32_t>;
					_stack.push_frame<frame_type::thread>(
					    returnTo - _story->instructions(), _evaluation_mode
					);
					_ref_stack.push_frame<frame_type::thread>(
					    returnTo - _story->instructions(), _evaluation_mode
					);

					// Fork a new thread on the callstack
					thread_t thread = _stack.fork_thread();
					{
						thread_t t = _ref_stack.fork_thread();
						inkAssert(t == thread, "ref_stack and stack should be in sync!");
					}

					if (debug_stream != nullptr) {
						*debug_stream << "thread " << thread;
					}

					// Push that thread onto our thread stack
					_threads.push(thread);
				} break;

				// == set temporary variable
				case Command::DEFINE_TEMP: {
					hash_t variableName = read<hash_t>();
					bool   is_redef     = flag & CommandFlag::ASSIGNMENT_IS_REDEFINE;

					if (debug_stream != nullptr) {
						*debug_stream << "variable_name ";
						write_hash(*debug_stream, variableName);
						*debug_stream << " is_redef " << (is_redef ? "yes" : "no");
					}

					// Get the top value and put it into the variable
					value v = _eval.pop();
					set_var<Scope::LOCAL>(variableName, v, is_redef);
				} break;

				case Command::SET_VARIABLE: {
					hash_t variableName = read<hash_t>();

					// Check if it's a redefinition (not yet used, seems important for pointers later?)
					bool is_redef = flag & CommandFlag::ASSIGNMENT_IS_REDEFINE;

					// If not, we're setting a global (temporary variables are explicitely defined as such,
					//  where globals are defined using SET_VARIABLE).
					value val = _eval.pop();

					if (debug_stream != nullptr) {
						*debug_stream << "variable_name ";
						write_hash(*debug_stream, variableName);
						*debug_stream << " is_redef " << (is_redef ? "yes" : "no");
					}

					if (is_redef) {
						set_var(variableName, val, is_redef);
					} else {
						set_var<Scope::GLOBAL>(variableName, val, is_redef);
					}
				} break;

				// == Function calls
				case Command::CALL_EXTERNAL: {
					// Read function name
					hash_t functionName = read<hash_t>();

					// Interpret flag as argument count
					int numArguments = ( int ) flag;

					if (debug_stream != nullptr) {
						*debug_stream << "function_name ";
						write_hash(*debug_stream, functionName);
						*debug_stream << " numArguments " << numArguments;
					}

					// find and execute. will automatically push a valid if applicable
					auto* fn = _functions.find(functionName);
					if (fn == nullptr) {
						_eval.push(values::ex_fn_not_found);
					} else if (_output.saved()
					           && _output.ends_with(value_type::newline, _output.save_offset())
					           && ! fn->lookaheadSafe()) {
						// TODO: seperate token?
						_output.append(values::null);
					} else {
						fn->call(&_eval, numArguments, _globals->strings(), _globals->lists());
					}
				} break;

				// == Evaluation stack
				case Command::START_EVAL: _evaluation_mode = true; break;
				case Command::END_EVAL:
					_evaluation_mode = false;

					// Assert stack is empty? Is that necessary?
					break;
				case Command::OUTPUT: {
					value v = _eval.pop();
					_output << v;
				} break;
				case Command::POP: _eval.pop(); break;
				case Command::DUPLICATE: _eval.push(_eval.top_value()); break;
				case Command::PUSH_VARIABLE_VALUE: {
					// Try to find in local stack
					hash_t       variableName = read<hash_t>();
					const value* val          = get_var(variableName);

					if (debug_stream != nullptr) {
						*debug_stream << "variable_name ";
						write_hash(*debug_stream, variableName);
						*debug_stream << " val \"" << val << "\"";
					}

					inkAssert(val != nullptr, "Could not find variable!");
					_eval.push(*val);
					break;
				}
				case Command::START_STR: {
					inkAssert(_evaluation_mode, "Can not enter string mode while not in evaluation mode!");
					_string_mode     = true;
					_evaluation_mode = false;
					_output << values::marker;
				} break;
				case Command::END_STR: {
					// TODO: Assert we really had a marker on there?
					inkAssert(! _evaluation_mode, "Must be in evaluation mode");
					_string_mode     = false;
					_evaluation_mode = true;

					// Load value from output stream
					// Push onto stack
					_eval.push(value{}.set<value_type::string>(
					    _output.get_alloc<false>(_globals->strings(), _globals->lists())
					));
				} break;

				// == Tag commands
				case Command::START_TAG: {
					_output << values::marker;
					_tag_mode = true;
				} break;

				case Command::TAG: {
					const char* tag = read<const char*>();

					if (debug_stream != nullptr) {
						*debug_stream << "tag \"" << tag << "\"";
					}

					add_tag(tag, _tags_where);
					_tag_mode = false;
				} break;

				case Command::END_TAG: {
					auto tag = _output.get_alloc<true>(_globals->strings(), _globals->lists());
					if (_string_mode && _choice_tags_begin == ~0) {
						_choice_tags_begin = _tags.size();
					}

					if (debug_stream != nullptr) {
						*debug_stream << "tag \"" << tag << "\"";
					}

					add_tag(tag, _tags_where);
					_tag_mode = false;
				} break;

				// == Choice commands
				case Command::CHOICE: {
					// Read path
					uint32_t path = read<uint32_t>();

					if (debug_stream != nullptr) {
						*debug_stream << "path " << path;
					}

					// If we're a once only choice, make sure our destination hasn't
					//  been visited
					if (flag & CommandFlag::CHOICE_IS_ONCE_ONLY) {
						// Need to convert offset to container index
						container_t destination = -1;
						if (_story->get_container_id(_story->instructions() + path, destination)) {
							// Ignore the choice if we've visited the destination before
							if (_globals->visits(destination) > 0) {
								break;
							}
						} else {
							inkAssert(false, "Destination for choice block does not have counting flags.");
						}
					}

					// Choice is conditional
					if (flag & CommandFlag::CHOICE_HAS_CONDITION) {
						// Only show if the top of the eval stack is 'truthy'
						if (! _eval.pop().truthy(_globals->lists())) {
							break;
						}
					}

					// Use a marker to start compiling the choice text
					_output << values::marker;
					value stack[2];
					int   sc = 0;

					if (flag & CommandFlag::CHOICE_HAS_START_CONTENT) {
						stack[sc++] = _eval.pop();
					}
					if (flag & CommandFlag::CHOICE_HAS_CHOICE_ONLY_CONTENT) {
						stack[sc++] = _eval.pop();
					}
					for (; sc; --sc) {
						_output << stack[sc - 1];
					}

					// Fetch tags related to the current choice
					const snap_tag* tags_start = nullptr;
					const snap_tag* tags_end   = nullptr;

					if (_choice_tags_begin != ~0) {
						// Retroactively mark added tags as choice tags
						const size_t tags_count = _tags.size() - _choice_tags_begin;
						_choice_tags_count += tags_count;
						_tags_where = tags_level::CHOICE;

						tags_start = _tags.begin() + _choice_tags_begin;
						tags_end   = tags_start + tags_count;

						_choice_tags_begin = ~0;
					}

					// Create choice and record it
					choice* current_choice = nullptr;
					if (flag & CommandFlag::CHOICE_IS_INVISIBLE_DEFAULT) {
						_fallback_choice.emplace();
						current_choice = &_fallback_choice.value();
					} else {
						current_choice = &add_choice();
					}
					current_choice->setup(
					    _output, _globals->strings(), _globals->lists(), _choices.size(), path,
					    current_thread(), tags_start, tags_end
					);
					// save stack at last choice
					if (_saved) {
						forget();
					}
					save();
				} break;
				case Command::START_CONTAINER_MARKER: {
					// Keep track of current container
					auto index = read<uint32_t>();
					// offset points to command, command has size 6
					_container.push({.id = index, .offset = _ptr - 6});

					// Increment visit count
					if (flag & CommandFlag::CONTAINER_MARKER_TRACK_VISITS
					    || flag & CommandFlag::CONTAINER_MARKER_TRACK_TURNS) {
						_globals->visit(_container.top().id, true);
					}

				} break;
				case Command::END_CONTAINER_MARKER: {
					container_t index = read<container_t>();

					inkAssert(_container.top().id == index, "Leaving container we are not in!");

					// Move up out of the current container
					_container.pop();

					// SPECIAL: If we've popped all containers, then there's an implied
					// 'done' command or return
					if (_container.empty()) {
						_is_falling = false;

						frame_type type;
						if (! _threads.empty()) {
							on_done(false);
							break;
						} else if (_stack.has_frame(&type) && type == frame_type::function) // implicit return
						                                                                    // is only for
						                                                                    // functions
						{
							// push null and return
							_eval.push(values::null);

							// HACK
							_ptr += sizeof(Command) + sizeof(CommandFlag);
							execute_return();
						} else if (_ptr == _story->end()) { // check needed, because it colud exist an unnamed
							                                  // toplevel container (empty named container stack
							                                  // != empty container stack)
							on_done(true);
						}
					}
				} break;
				case Command::VISIT: {
					// Push the visit count for the current container to the top
					//  is 0-indexed for some reason. idk why but this is what ink expects
					_eval.push(
					    value{}.set<value_type::int32>(( int ) _globals->visits(_container.top().id) - 1)
					);
				} break;
				case Command::TURN: {
					_eval.push(value{}.set<value_type::int32>(( int ) _globals->turns()));
				} break;
				case Command::SEQUENCE: {
					// TODO: The C# ink runtime does a bunch of fancy logic
					//  to make sure each element is picked at least once in every
					//  iteration loop. I don't feel like replicating that right now.
					// So, let's just return a random number and *shrug*
					int sequenceLength = _eval.pop().get<value_type::int32>();
					int index          = _eval.pop().get<value_type::int32>();

					_eval.push(value{}.set<value_type::int32>(static_cast<int32_t>(_rng.rand(sequenceLength)))
					);
				} break;
				case Command::SEED: {
					int32_t seed = _eval.pop().get<value_type::int32>();
					_rng.srand(seed);

					if (debug_stream != nullptr) {
						*debug_stream << "seed " << seed;
					}

					_eval.push(values::null);
				} break;

				case Command::READ_COUNT: {
					// Get container index
					container_t container = read<container_t>();

					// Push the read count for the requested container index
					_eval.push(value{}.set<value_type::int32>(( int ) _globals->visits(container)));
				} break;

				default: inkAssert(false, "Unrecognized command!"); break;
			}
		}

		if (debug_stream != nullptr) {
			*debug_stream << std::endl;
		}
	}
#ifndef INK_ENABLE_UNREAL
	catch (...) {
		// Reset our whole state as it's probably corrupt
		reset();
		throw;
	}
#endif
}

void runner_impl::on_done(bool setDone)
{
	// If we're in a thread
	if (! _threads.empty()) {
		// Get the thread ID of the current thread
		thread_t completedThreadId = _threads.pop();

		// Push in a complete marker
		_stack.complete_thread(completedThreadId);
		_ref_stack.complete_thread(completedThreadId);

		// Go to where the thread started
		frame_type type = execute_return();
		inkAssert(
		    type == frame_type::thread,
		    "Expected thread frame marker to hold return to value but none found..."
		);
		// if thread ends, move stave point with, else the thread end marker is missing
		// and we can't collect the other threads
		if (_saved) {
			forget();
			save();
		}
	} else {
		if (setDone) {
			set_done_ptr(_ptr);
		}
		_ptr = nullptr;
	}
}

void runner_impl::set_done_ptr(ip_t ptr)
{
	thread_t curr = current_thread();
	if (curr == ~0) {
		_done = ptr;
	} else {
		_threads.set(curr, ptr);
	}
}

void runner_impl::reset()
{
	_eval.clear();
	_output.clear();
	_stack.clear();
	_ref_stack.clear();
	_threads.clear();
	_evaluation_mode = false;
	_saved           = false;
	_choices.clear();
	_ptr  = nullptr;
	_done = nullptr;
	_container.clear();
}

void runner_impl::mark_used(string_table& strings, list_table& lists) const
{
	// Find strings in output and stacks
	_output.mark_used(strings, lists);
	_stack.mark_used(strings, lists);
	// ref_stack has no strings and lists!
	_eval.mark_used(strings, lists);

	// Take into account tags
	for (size_t i = 0; i < _tags.size(); ++i) {
		strings.mark_used(_tags[i]);
	}
	// Take into account choice text
	for (size_t i = 0; i < _choices.size(); i++) {
		strings.mark_used(_choices[i]._text);
	}
}

void runner_impl::save()
{
	inkAssert(! _saved, "Runner state already saved");

	_saved = true;
	_output.save();
	_stack.save();
	_ref_stack.save();
	_backup = _ptr;
	_container.save();
	_globals->save();
	_eval.save();
	_threads.save();
	_choices.save();
	_tags.save();
	_saved_evaluation_mode = _evaluation_mode;

	// Not doing this anymore. There can be lingering stack entries from function returns
	// inkAssert(_eval.is_empty(), "Can not save interpreter state while eval stack is not empty");
}

void runner_impl::restore()
{
	inkAssert(_saved, "Can't restore. No runner state saved.");
	// the output can be restored without the rest
	if (_output.saved()) {
		_output.restore();
	}
	_stack.restore();
	_ref_stack.restore();
	_ptr = _backup;
	_container.restore();
	_globals->restore();
	_eval.restore();
	_threads.restore();
	_choices.restore();
	_tags.restore();
	_evaluation_mode = _saved_evaluation_mode;

	// Not doing this anymore. There can be lingering stack entries from function returns
	// inkAssert(_eval.is_empty(), "Can not save interpreter state while eval stack is not empty");

	_saved = false;
}

void runner_impl::forget()
{
	// Do nothing if we haven't saved
	if (! _saved) {
		return;
	}

	_output.forget();
	_stack.forget();
	_ref_stack.forget();
	_container.forget();
	_globals->forget();
	_eval.forget();
	_threads.forget();
	_choices.forgett();
	_tags.forgett();

	// Nothing to do for eval stack. It should just stay as it is

	_saved = false;
}

#ifdef INK_ENABLE_STL
std::ostream& operator<<(std::ostream& out, runner_impl& in)
{
	in.getline(out);
	return out;
}
#endif

} // namespace ink::runtime::internal
