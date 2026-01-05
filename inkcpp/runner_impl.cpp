/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "runner_impl.h"

#include "choice.h"
#include "command.h"
#include "globals_impl.h"
#include "header.h"
#include "snapshot_impl.h"
#include "story_impl.h"
#include "system.h"
#include "value.h"

#include <iomanip>
#include <vector>

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

size_t runner_interface::num_choices() const { return static_cast<size_t>(end() - begin()); }
} // namespace ink::runtime

namespace ink::runtime::internal
{

hash_t runner_impl::get_current_knot() const
{
	return _current_knot_id == ~0U ? 0 : _story->container_hash(_current_knot_id);
}

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
	int end      = static_cast<int>(where) + 1;
	int position = _tags_begin[end];
	for (size_t i = end; i < _tags_begin.capacity(); ++i) {
		_tags_begin.set(i, _tags_begin[i] + 1);
	}
	return _tags.insert(position) = value;
}

void runner_impl::copy_tags(tags_level src, tags_level dst)
{
	inkAssert(dst < src, "Only support copieng to higher state!");
	size_t old_size = _tags.size();
	size_t idx      = _tags_begin[static_cast<int>(src)];
	size_t n        = _tags_begin[static_cast<int>(src) + 1] - idx;
	size_t idy      = _tags_begin[static_cast<int>(dst) + 1];
	for (size_t i = 0; i < n; ++i) {
		_tags.insert(idy + n) = _tags[idx + i * 2];
	}
	for (size_t i = static_cast<int>(dst) + 1; i < _tags_begin.capacity(); ++i) {
		_tags_begin.set(i, _tags_begin[i] + n);
	}
	inkAssert(_tags.size() == old_size + n, "Expected n new elements");
}

void runner_impl::assign_tags(std::initializer_list<tags_level> wheres)
{
	size_t old_size = _tags.size();
	size_t idy      = _tags_begin[static_cast<int>(tags_level::UNKNOWN)];
	size_t end      = _tags_begin[static_cast<int>(tags_level::UNKNOWN) + 1];
	size_t n        = end - idy;
	if (n == 0) {
		return;
	}
	for (const tags_level& where : wheres) {
		size_t idx = _tags_begin[static_cast<int>(where) + 1];
		idy        = _tags_begin[static_cast<int>(tags_level::UNKNOWN)];
		end        = _tags_begin[static_cast<int>(tags_level::UNKNOWN) + 1];
		inkAssert(n == end - idy, "Same size in each iteration");
		for (size_t i = 0; i < n; ++i) {
			const char* tag       = _tags[idy + i * 2];
			_tags.insert(idx + i) = tag;
		}
		for (size_t i = static_cast<int>(where) + 1; i < _tags_begin.capacity(); ++i) {
			_tags_begin.set(i, _tags_begin[i] + n);
		}
	}
	_tags_begin.set(
	    static_cast<int>(tags_level::UNKNOWN) + 1, _tags_begin[static_cast<int>(tags_level::UNKNOWN)]
	);
	_tags.resize(_tags_begin[static_cast<int>(tags_level::UNKNOWN)]);

	inkAssert(_tags.size() == old_size + (wheres.size() - 1) * n, "Expected to preserve size!");
}

void runner_impl::clear_tags(tags_clear_level which)
{
	// clear all tags begin which we do not want to keep
	//
	// resize to first other field which begins which should not be cleared
	switch (which) {
		case tags_clear_level::KEEP_NONE:
			_tags.clear();
			for (size_t i = 0; i < _tags_begin.capacity(); ++i) {
				_tags_begin.set(i, 0);
			}
			break;
		case tags_clear_level::KEEP_GLOBAL_AND_UNKNOWN:
			_tags.remove(
			    _tags_begin[static_cast<int>(tags_level::KNOT)],
			    _tags_begin[static_cast<int>(tags_level::UNKNOWN)]
			);
			for (size_t i = static_cast<int>(tags_level::KNOT); i < _tags_begin.capacity(); ++i) {
				_tags_begin.set(i, _tags_begin[static_cast<int>(tags_level::KNOT)]);
			}
			_tags_begin.set(static_cast<int>(tags_level::UNKNOWN) + 1, _tags.size());
			break;
		case tags_clear_level::KEEP_KNOT:
			_tags.resize(_tags_begin[static_cast<int>(tags_level::KNOT) + 1]);
			for (size_t i = static_cast<int>(tags_level::KNOT) + 2; i < _tags_begin.capacity(); ++i) {
				_tags_begin.set(i, _tags_begin[static_cast<int>(tags_level::KNOT) + 1]);
			}
			break;
		default: inkAssert(false, "Unhandeld clear type %d for tags.", static_cast<int>(which));
	}
}

void runner_impl::jump(ip_t dest, bool record_visits, bool track_knot_visit)
{
	// Optimization: if we are _is_falling, then we can
	//  _should be_ able to safely assume that there is nothing to do here. A falling
	//  divert should only be taking us from a container to that same container's end point
	//  without entering any other containers
	// OR IF is target is same position do nothing
	// could happened if jumping to and of an unnamed container
	if (dest == _ptr) {
		_ptr = dest;
		return;
	}

	const uint32_t* iter = nullptr;
	container_t     id;
	ip_t            offset   = nullptr;
	bool            reversed = _ptr > dest;

	// move to destination and update container stack on the go
	const ContainerData* c_iter   = nullptr;
	// number of container which were already on the stack at current position
	size_t               comm_end = _container.size();

	iter = nullptr;
	while (_story->iterate_containers(iter, id, offset)) {
		if (offset >= _ptr) {
			break;
		}
	}
	if (! reversed) {
		_story->iterate_containers(iter, id, offset, true);
	}
	optional<ContainerData> last_pop = nullopt;
	while (_story->iterate_containers(iter, id, offset, reversed)) {
		if ((! reversed && offset >= dest) || (reversed && offset < dest)) {
			break;
		}
		if (_container.empty() || _container.top().id != id) {
			const uint32_t* iter2 = nullptr;
			container_t     id2;
			ip_t            offset2;
			while (_story->iterate_containers(iter2, id2, offset2) && id2 != id) {}
			_container.push({id, offset2 - _story->instructions()});
		} else {
			if (_container.size() == comm_end) {
				last_pop = _container.pop();
				comm_end -= 1;
			} else {
				_container.pop();
			}
		}
	}
	iter = nullptr;
	while (_story->iterate_containers(iter, id, offset)) {
		if (offset >= dest) {
			break;
		}
	}

	// if we jump directly to a named container start, go inside, if it's a ONLY_FIRST container
	// it will get visited in the next step
	// todo: check if a while is needed
	if (offset == dest && static_cast<Command>(offset[0]) == Command::START_CONTAINER_MARKER) {
		if (track_knot_visit
		    && static_cast<CommandFlag>(offset[1]) & CommandFlag::CONTAINER_MARKER_IS_KNOT) {
			_current_knot_id = id;
			_entered_knot    = true;
		}
		dest += 6;
		_container.push({id, offset - _story->instructions()});
		// if we entered a knot we just left, do not recount enter
		if (reversed && comm_end == _container.size() - 1 && last_pop.has_value()
		    && last_pop.value().id == id) {
			comm_end += 1;
		}
	}
	_ptr = dest;

	// iff all container (until now) are entered at first position
	bool allEnteredAtStart = true;
	ip_t child_position    = dest;
	if (record_visits) {
		const ContainerData* iData = nullptr;
		size_t               level = _container.size();
		while (_container.iter(iData)) {
			if (level > comm_end
			    || _story->container_flag(iData->offset + _story->instructions())
			           & CommandFlag::CONTAINER_MARKER_ONLY_FIRST) {
				auto parrent_offset = _story->instructions() + iData->offset;
				inkAssert(child_position >= parrent_offset, "Container stack order is broken");
				// 6 == len of START_CONTAINER_SIGNAL, if its 6 bytes behind the container it is a
				// unnnamed subcontainers first child check if child_positino is the first child of
				// current container
				allEnteredAtStart = allEnteredAtStart && ((child_position - parrent_offset) <= 6);
				child_position    = parrent_offset;
				_globals->visit(iData->id, allEnteredAtStart);
			}
			level -= 1;
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
		offset_t address = static_cast<offset_t>(_ptr - _story->instructions());
		_stack.push_frame<type>(address, _evaluation_mode);
		_ref_stack.push_frame<type>(address, _evaluation_mode);
	}
	_evaluation_mode = false; // unset eval mode when enter function or tunnel

	// Do the jump
	inkAssert(_story->instructions() + target < _story->end(), "Diverting past end of story data!");
	jump(_story->instructions() + target, true, false);
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
	jump(_story->instructions() + offset, false, false);

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
    , _backup(nullptr)
    , _done(nullptr)
    , _evaluation_mode{false}
    , _choices()
    , _tags_begin(0, ~0)
    , _container(ContainerData{})
    , _rng(static_cast<uint32_t>(time(NULL)))
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

#if defined(INK_ENABLE_STL) || defined(INK_ENABLE_UNREAL)

runner_impl::line_type runner_impl::getline()
{
	// Advance interpreter one line and write to output
	advance_line();

#	ifdef INK_ENABLE_STL
	line_type result{_output.get()};
#	elif defined(INK_ENABLE_UNREAL)
	line_type result{ANSI_TO_TCHAR(_output.get_alloc(_globals->strings(), _globals->lists()))};
#	else
#		error unsupported constraints for getline
#	endif

	// Fall through the fallback choice, if available
	if (! has_choices() && _fallback_choice) {
		choose(~0U);
	}
	inkAssert(_output.is_empty(), "Output should be empty after getline!");

	return result;
}

runner_impl::line_type runner_impl::getall()
{
#	ifdef INK_ENABLE_STL
	if (_debug_stream != nullptr) {
		_debug_stream->clear();
	}
#	endif

	line_type result{};

	while (can_continue()) {
		result += getline();
	}
	inkAssert(_output.is_empty(), "Output should be empty after getall!");

	return result;
}

#endif

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

void runner_impl::advance_line()
{
	clear_tags(tags_clear_level::KEEP_KNOT);

	// Step while we still have instructions to execute
	while (_ptr != nullptr) {
		// Stop if we hit a new line
		if (line_step()) {
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
	} else if (! _fallback_choice) {
		inkAssert(false, "No choice and no Fallbackchoice!! can not choose");
	}
	_globals->turn();
	// Get the choice
	const auto& c = has_choices() ? _choices[index] : _fallback_choice.value();

	// Get its thread
	thread_t choiceThread = c._thread;

	// Figure out where our previous pointer was for that thread
	ip_t prev = nullptr;
	if (choiceThread == ~0U) {
		prev = _done;
	} else {
		prev = _threads.get(choiceThread);
	}

	// Make sure we have a previous pointer
	inkAssert(prev != nullptr, "No 'done' point recorded before finishing choice output");

	// Move to the previous pointer so we track our movements correctly
	jump(prev, false, false);
	_done = nullptr;

	// Collapse callstacks to the correct thread
	_stack.collapse_to_thread(choiceThread);
	_ref_stack.collapse_to_thread(choiceThread);
	_threads.clear();
	_eval.clear();

	// Jump to destination and clear choice list
	jump(_story->instructions() + c.path(), true, false);
	clear_choices();
	_entered_knot = false;
}

void runner_impl::getline_silent()
{
	// advance and clear output stream
	advance_line();
	_output.clear();
}

snapshot* runner_impl::create_snapshot() const { return _globals->create_snapshot(); }

size_t runner_impl::snap(unsigned char* data, snapper& snapper) const
{
	unsigned char* ptr          = data;
	bool           should_write = data != nullptr;
	std::uintptr_t offset       = _ptr != nullptr ? _ptr - _story->instructions() : 0;
	ptr                         = snap_write(ptr, offset, should_write);
	offset                      = _backup - _story->instructions();
	ptr                         = snap_write(ptr, offset, should_write);
	offset                      = _done - _story->instructions();
	ptr                         = snap_write(ptr, offset, should_write);
	ptr                         = snap_write(ptr, _rng.get_state(), should_write);
	ptr                         = snap_write(ptr, _evaluation_mode, should_write);
	ptr                         = snap_write(ptr, _string_mode, should_write);
	ptr                         = snap_write(ptr, _saved_evaluation_mode, should_write);
	ptr                         = snap_write(ptr, _saved, should_write);
	ptr                         = snap_write(ptr, _is_falling, should_write);
	ptr += _output.snap(data ? ptr : nullptr, snapper);
	ptr += _stack.snap(data ? ptr : nullptr, snapper);
	ptr += _ref_stack.snap(data ? ptr : nullptr, snapper);
	ptr += _eval.snap(data ? ptr : nullptr, snapper);
	ptr += _tags_begin.snap(data ? ptr : nullptr, snapper);
	ptr += _tags.snap(data ? ptr : nullptr, snapper);
	snapper.runner_tags = _tags.data();
	ptr                 = snap_write(ptr, _entered_global, should_write);
	ptr                 = snap_write(ptr, _entered_knot, should_write);
	ptr                 = snap_write(ptr, _current_knot_id, should_write);
	ptr                 = snap_write(ptr, _current_knot_id_backup, should_write);
	ptr += _container.snap(data ? ptr : nullptr, snapper);
	ptr += _threads.snap(data ? ptr : nullptr, snapper);
	ptr = snap_write(ptr, _fallback_choice.has_value(), should_write);
	if (_fallback_choice) {
		ptr += _fallback_choice.value().snap(data ? ptr : nullptr, snapper);
	}
	ptr += _choices.snap(data ? ptr : nullptr, snapper);
	return static_cast<size_t>(ptr - data);
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
	ptr                = snap_read(ptr, _saved_evaluation_mode);
	ptr                = snap_read(ptr, _saved);
	ptr                = snap_read(ptr, _is_falling);
	ptr                = _output.snap_load(ptr, loader);
	ptr                = _stack.snap_load(ptr, loader);
	ptr                = _ref_stack.snap_load(ptr, loader);
	ptr                = _eval.snap_load(ptr, loader);
	ptr                = _tags_begin.snap_load(ptr, loader);
	ptr                = _tags.snap_load(ptr, loader);
	loader.runner_tags = _tags.data();
	ptr                = snap_read(ptr, _entered_global);
	ptr                = snap_read(ptr, _entered_knot);
	ptr                = snap_read(ptr, _current_knot_id);
	ptr                = snap_read(ptr, _current_knot_id_backup);
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
	advance_line();
	const char* res = _output.get_alloc(_globals->strings(), _globals->lists());
	if (! has_choices() && _fallback_choice) {
		choose(~0U);
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
	jump(destination, false, true);

	return true;
}

void runner_impl::internal_bind(hash_t name, internal::function_base* function)
{
	_functions.add(name, function);
}

runner_impl::change_type runner_impl::detect_change() const
{
	inkAssert(_output.saved(), "Cannot detect changes in non-saved stream.");

	// Check if the old newline is still present (hasn't been glu'd) and
	//  if there is new text (non-whitespace) in the stream since saving
	bool stillHasNewline = _output.ends_with(value_type::newline, _output.save_offset());
	bool hasAddedNewText = _output.text_past_save() || _tags.last_size() < _tags.size();

	// Newline is still there and there's no new text
	if (stillHasNewline && ! hasAddedNewText) {
		return change_type::no_change;
	}

	// If the newline is gone, we got glue'd. Continue as if we never had that newline
	if (! stillHasNewline) {
		return change_type::newline_removed;
	}

	// If there's new text content, we went too far
	if (hasAddedNewText) {
		return change_type::extended_past_newline;
	}
	inkFail("Invalid change detction. Never should be here!");
	return change_type::no_change;
}

bool runner_impl::line_step()
{
	// Track if added tags are global ones
	if (_ptr == _story->instructions()) {

		// Step the interpreter until we've parsed all tags for the line
		_entered_global  = true;
		_current_knot_id = ~0U;
		_entered_knot    = false;
	}
	// Step the interpreter
	// Copy global tags to the first line
	size_t o_size = _output.filled();
	step();
	if ((o_size < _output.filled() && _output.find_first_of(value_type::marker) == _output.npos
	     && ! _evaluation_mode && ! _saved)
	    || (_entered_knot && _entered_global)) {
		if (_entered_global) {
			assign_tags({tags_level::LINE, tags_level::GLOBAL});
			_entered_global = false;
		} else if (_entered_knot) {
			if (has_knot_tags()) {
				clear_tags(tags_clear_level::KEEP_GLOBAL_AND_UNKNOWN
				); // clear knot tags since whe are entering another knot
			}

			// Next tags are always added to the line
			assign_tags({tags_level::LINE, tags_level::KNOT});

			// Unless we are out of content, we are going to try
			//  to continue a little further. This is to check for
			//  glue (which means there is potentially more content
			//  in this line) OR for non-text content such as choices.
			// Save a snapshot
			_entered_knot = false;
		}
	}

	// If we're not within string evaluation
	if (_output.find_first_of(value_type::marker) == _output.npos) {

		// Haven't added more text


		// If we have a saved state after a previous newline
		// don't do this if we behind choice
		if (_saved && ! has_choices() && ! _fallback_choice) {
			// Check for changes in the output stream
			switch (detect_change()) {
				case change_type::extended_past_newline:
					// We've gone too far. Restore to before we moved past the newline and return that we are
					// done
					restore();
					assign_tags({tags_level::LINE});
					return true;
				case change_type::newline_removed:
					// Newline was removed. Proceed as if we never hit it
					forget();
					break;
				case change_type::no_change: break;
			}
		}

		// If we're on a newline
		if (_output.ends_with(value_type::newline)) {
			// Unless we are out of content, we are going to try
			// to continue a little further. This is to check for
			// glue (which means there is potentially more content
			// in this line) OR for non-text content such as choices.
			if (_ptr != nullptr) {
				// Save a snapshot of the current runtime state so we
				// can return here if we end up hitting a new line
				if (! _saved) {
					assign_tags({tags_level::LINE});
					save();
				}
			}
			// Otherwise, make sure we don't have any snapshots hanging around
			// expect we are in choice handleing
			else if (! has_choices() && ! _fallback_choice) {
				forget();
			} else {
				_output.forget();
			}
		}
	}

	return false;
}

void runner_impl::step()
{
#ifdef INK_ENABLE_EXCEPTIONS
	try
#endif
	{
		inkAssert(_ptr != nullptr, "Can not step! Do not have a valid pointer");

		// Load current command
		Command     cmd  = read<Command>();
		CommandFlag flag = read<CommandFlag>();

#ifdef INK_ENABLE_STL
		if (_debug_stream != nullptr) {
			*_debug_stream << "cmd " << cmd << " flags " << flag << " ";
		}
#endif

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

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "str \"" << str << "\"";
					}
#endif

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::string>(str));
					} else {
						_output << value{}.set<value_type::string>(str);
					}
				} break;
				case Command::INT: {
					int val = read<int>();

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "int " << val;
					}
#endif

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::int32>(static_cast<int32_t>(val)));
					}
					// TEST-CASE B006 don't print integers
				} break;
				case Command::BOOL: {
					bool val = read<int>() ? true : false;

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "bool " << (val ? "true" : "false");
					}
#endif

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::boolean>(val));
					} else {
						_output << value{}.set<value_type::boolean>(val);
					}
				} break;
				case Command::FLOAT: {
					float val = read<float>();

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "float " << val;
					}
#endif

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::float32>(val));
					}
					// TEST-CASE B006 don't print floats
				} break;
				case Command::VALUE_POINTER: {
					hash_t val = read<hash_t>();

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "value_pointer ";
						write_hash(*_debug_stream, val);
					}
#endif

					if (_evaluation_mode) {
						_eval.push(value{}.set<value_type::value_pointer>(val, static_cast<char>(flag) - 1));
					} else {
						inkFail("never conciderd what should happend here! (value pointer print)");
					}
				} break;
				case Command::LIST: {
					list_table::list list(read<int>());

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "list " << list.lid;
					}
#endif

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

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "target " << target;
					}
#endif

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
					jump(_story->instructions() + target, true, ! (flag & CommandFlag::DIVERT_HAS_CONDITION));
				} break;
				case Command::DIVERT_TO_VARIABLE: {
					// Get variable value
					hash_t variable = read<hash_t>();

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "variable ";
						write_hash(*_debug_stream, variable);
					}
#endif

					// Check for condition
					if (flag & CommandFlag::DIVERT_HAS_CONDITION && ! _eval.pop().truthy(_globals->lists())) {
						break;
					}

					const value* val = get_var(variable);
					inkAssert(val, "Jump destiniation needs to be defined!");

					// Move to location
					jump(
					    _story->instructions() + val->get<value_type::divert>(), true,
					    ! (flag & CommandFlag::DIVERT_HAS_CONDITION)
					);
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

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "target " << target;
					}
#endif

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

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "target " << target;
					}
#endif

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
					    static_cast<offset_t>(returnTo - _story->instructions()), _evaluation_mode
					);
					_ref_stack.push_frame<frame_type::thread>(
					    static_cast<offset_t>(returnTo - _story->instructions()), _evaluation_mode
					);

					// Fork a new thread on the callstack
					thread_t thread = _stack.fork_thread();
					{
						thread_t t = _ref_stack.fork_thread();
						inkAssert(t == thread, "ref_stack and stack should be in sync!");
					}

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "thread " << thread;
					}
#endif

					// Push that thread onto our thread stack
					_threads.push(thread);
				} break;

				// == set temporärie variable
				case Command::DEFINE_TEMP: {
					hash_t variableName = read<hash_t>();
					bool   is_redef     = flag & CommandFlag::ASSIGNMENT_IS_REDEFINE;

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "variable_name ";
						write_hash(*_debug_stream, variableName);
						*_debug_stream << " is_redef " << (is_redef ? "yes" : "no");
					}
#endif

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

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "variable_name ";
						write_hash(*_debug_stream, variableName);
						*_debug_stream << " is_redef " << (is_redef ? "yes" : "no");
					}
#endif

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
					int numArguments = static_cast<int>(flag);

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "function_name ";
						write_hash(*_debug_stream, functionName);
						*_debug_stream << " numArguments " << numArguments;
					}
#endif

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

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "variable_name ";
						write_hash(*_debug_stream, variableName);
						*_debug_stream << " val \"" << val << "\"";
					}
#endif

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
				} break;


				case Command::END_TAG: {
					auto tag = _output.get_alloc<true>(_globals->strings(), _globals->lists());
					add_tag(tag, tags_level::UNKNOWN);
				} break;

				// == Choice commands
				case Command::CHOICE: {
					// Read path
					uint32_t path = read<uint32_t>();

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "path " << path;
					}
#endif

					// If we're a once only choice, make sure our destination hasn't
					//  been visited
					if (flag & CommandFlag::CHOICE_IS_ONCE_ONLY) {
						// Need to convert offset to container index
						container_t destination = ~0U;
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

					size_t start = _tags_begin[static_cast<int>(tags_level::CHOICE) + 1];
					assign_tags({tags_level::CHOICE});
					const snap_tag* tags_start = _tags.data() + start;
					const snap_tag* tags_end
					    = _tags.data() + _tags_begin[static_cast<int>(tags_level::CHOICE) + 1];

					// Create choice and record it
					choice* current_choice = nullptr;
					if (flag & CommandFlag::CHOICE_IS_INVISIBLE_DEFAULT) {
						_fallback_choice.emplace();
						current_choice = &_fallback_choice.value();
					} else {
						current_choice = &add_choice();
					}
					current_choice->setup(
					    _output, _globals->strings(), _globals->lists(), _choices.size() - 1, path,
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
					_container.push({index, _ptr - _story->instructions() - 6});

					// Increment visit count
					if (flag & CommandFlag::CONTAINER_MARKER_TRACK_VISITS
					    || flag & CommandFlag::CONTAINER_MARKER_TRACK_TURNS) {
						_globals->visit(_container.top().id, true);
					}
					if (flag & CommandFlag::CONTAINER_MARKER_IS_KNOT) {
						_current_knot_id = index;
						_entered_knot    = true;
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
					_eval.push(value{}.set<value_type::int32>(
					    static_cast<int32_t>(_globals->visits(_container.top().id) - 1)
					));
				} break;
				case Command::TURN: {
					_eval.push(value{}.set<value_type::int32>(static_cast<int32_t>(_globals->turns())));
				} break;
				case Command::SEQUENCE: {
					// TODO: The C# ink runtime does a bunch of fancy logic
					//  to make sure each element is picked at least once in every
					//  iteration loop. I don't feel like replicating that right now.
					// So, let's just return a random number and *shrug*
					int sequenceLength = _eval.pop().get<value_type::int32>();
					/* shuffel index */
					_eval.pop();


					_eval.push(value{}.set<value_type::int32>(static_cast<int32_t>(_rng.rand(sequenceLength)))
					);
				} break;
				case Command::SEED: {
					int32_t seed = _eval.pop().get<value_type::int32>();
					_rng.srand(seed);

#ifdef INK_ENABLE_STL
					if (_debug_stream != nullptr) {
						*_debug_stream << "seed " << seed;
					}
#endif

					_eval.push(values::null);
				} break;

				case Command::READ_COUNT: {
					// Get container index
					container_t container = read<container_t>();

					// Push the read count for the requested container index
					_eval.push(value{}.set<value_type::int32>(static_cast<int32_t>(_globals->visits(container)
					)));
				} break;
				case Command::TAG: {
					add_tag(read<const char*>(), tags_level::UNKNOWN);
				} break;
				default: inkAssert(false, "Unrecognized command!"); break;
			}
		}

#ifdef INK_ENABLE_STL
		if (_debug_stream != nullptr) {
			*_debug_stream << std::endl;
		}
#endif
	}
#ifdef INK_ENABLE_EXCEPTIONS
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
	if (curr == ~0U) {
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
	_tags_begin.save();
	_saved_evaluation_mode  = _evaluation_mode;
	_current_knot_id_backup = _current_knot_id;

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
	_tags_begin.restore();
	_evaluation_mode        = _saved_evaluation_mode;
	_current_knot_id        = _current_knot_id_backup;
	_current_knot_id_backup = ~0U;
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
	_tags_begin.forget();
	_current_knot_id_backup = ~0U;
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

config::statistics::runner runner_impl::statistics() const
{
	return {_threads.statistics(), _eval.statistics(),   _container.statistics(),
	        _tags.statistics(),    _stack.statistics(),  _ref_stack.statistics(),
	        _output.statistics(),  _choices.statistics()};
}

} // namespace ink::runtime::internal
