#include "runner_impl.h"
#include "story_impl.h"
#include "command.h"
#include "choice.h"
#include "globals_impl.h"
#include "header.h"
#include "string_utils.h"

namespace ink::runtime
{
	const choice* runner_interface::get_choice(size_t index) const
	{
		inkAssert(index < num_choices(), "Choice out of bounds!");
		return begin() + index;
	}

	size_t runner_interface::num_choices() const
	{
		return end() - begin();
	}
}

namespace ink::runtime::internal
{

	template<>
	value* runner_impl::get_var<runner_impl::Scope::GLOBAL>(hash_t variableName) {
		return _globals->get_variable(variableName);
	}

	template<>
	value* runner_impl::get_var<runner_impl::Scope::LOCAL>(hash_t variableName) {
		value* ret = _stack.get(variableName);
		if(!ret) { return nullptr; }
		if(ret->type() == value_type::value_pointer) {
			auto [name, ci] = ret->get<value_type::value_pointer>();
			inkAssert(ci == 0, "only Global pointer are allowd on the _stack!");
			return get_var<runner_impl::Scope::GLOBAL>(name);
		}
		return ret;
	}

	template<>
	value* runner_impl::get_var<runner_impl::Scope::NONE>(hash_t variableName) {
		value* ret = get_var<Scope::LOCAL>(variableName);
		if(ret) { return ret; }
		return get_var<Scope::GLOBAL>(variableName);
	}
	template<runner_impl::Scope Hint>	
	const value* runner_impl::get_var(hash_t variableName) const {
		return const_cast<runner_impl*>(this)->get_var<Hint>(variableName);
	}

	template<>
	void runner_impl::set_var<runner_impl::Scope::GLOBAL>(hash_t variableName, const value& val, bool is_redef) {
		if(is_redef) {
			value* src = _globals->get_variable(variableName);
			_globals->set_variable(variableName, src->redefine(val, _globals->lists()));
		} else {
			_globals->set_variable(variableName, val);
		}
	}

	const value* runner_impl::dereference(const value& val) {
		if(val.type() != value_type::value_pointer) { return &val; }
		
		auto [name, ci] = val.get<value_type::value_pointer>();
		if(ci == 0) { return get_var<Scope::GLOBAL>(name); }
		return _stack.get_from_frame(ci, name);
	}

	template<>
	void runner_impl::set_var<runner_impl::Scope::LOCAL>(hash_t variableName, const value& val, bool is_redef) {
		if(val.type() == value_type::value_pointer) {
			inkAssert(is_redef == false, "value pointer can only use to initelize variable!");
			auto [name, ci] = val.get<value_type::value_pointer>();
			if(ci == 0) { _stack.set(variableName, val); }
			else {
				const value* dref = dereference(val);
				if(dref == nullptr) {
					value v = val;
					auto ref = v.get<value_type::value_pointer>();
					v.set<value_type::value_pointer>(ref.name, 0);
					_stack.set(variableName, v);
				} else {
					_ref_stack.set(variableName, val);
					_stack.set(variableName, *dref);
				}
			}
		} else {
			if(is_redef) {
				value* src = _stack.get(variableName);
				if(src->type() == value_type::value_pointer) {
					auto [name, ci] = src->get<value_type::value_pointer>();
					inkAssert(ci == 0, "Only global pointer are allowed on _stack!");
					set_var<Scope::GLOBAL>(
							name,
							get_var<Scope::GLOBAL>(name)->redefine(val, _globals->lists()),
							true);
				} else {
					_stack.set(variableName, src->redefine(val, _globals->lists()));
				}
			} else {
				_stack.set(variableName, val);
			}
		}
	}

	template<>
	void runner_impl::set_var<runner_impl::Scope::NONE>(hash_t variableName, const value& val, bool is_redef) 	
	{
		inkAssert(is_redef, "define set scopeless variables!");
		if(_stack.get(variableName)) {
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
		T val = *(const T*)_ptr;
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
		inkAssert(config::maxChoices < 0 || _choices.size() < config::maxChoices,
				"Ran out of choice storage!");
		return _choices.push();
	}

	void runner_impl::clear_choices()
	{
		// TODO: Garbage collection? ? which garbage ?
		_fallback_choice = nullopt;
		_choices.clear();
	}

	void runner_impl::clear_tags()
	{
		_tags.clear();
	}

	void runner_impl::jump(ip_t dest, bool record_visits)
	{
		// Optimization: if we are _is_falling, then we can
		//  _should be_ able to safely assume that there is nothing to do here. A falling
		//  divert should only be taking us from a container to that same container's end point
		//  without entering any other containers
		/*if (_is_falling)
		{
			_ptr = dest;
			return;
		}*/

		// Check which direction we are jumping
		bool reverse = dest < _ptr;

		// iteration
		const uint32_t* iter = nullptr;
		container_t container_id;
		ip_t offset;
		bool inBound = false;

		// Iterate until we find the container marker just before our own
		while (_story->iterate_containers(iter, container_id, offset, reverse)) {
			if (( !reverse && offset > _ptr )
					|| ( reverse && offset < _ptr )) {

				// Step back once in the iteration and break
				inBound = true;
				_story->iterate_containers(iter, container_id, offset, !reverse);
				break;
			}
		}

		size_t pos = _container.size();

		bool first = true;
		// Start moving forward (or backwards)
		if(inBound && (offset == nullptr || (!reverse&&offset<=dest) || (reverse&&offset>dest)) )
			while (_story->iterate_containers(iter, container_id, offset, reverse))
			{
				// Break when we've past the destination
				if ((!reverse && offset > dest) || (reverse && offset <= dest)) {
					// jump back to start of same container
					if(first && reverse && offset == dest
							&& _container.top() == container_id)  {
						// check if it was start flag
						auto con_id = container_id;
						_story->iterate_containers(iter, container_id, offset, true);
						if(offset == nullptr || con_id == container_id) 
						{
							_globals->visit(container_id);
						}
					}
					break;
				}
				first = false;

				// Two cases:

				// (1) Container iterator has the same value as the top of the stack.
				//  This means that this is an end marker for the container we're in
				if (!_container.empty() && _container.top() == container_id)
				{
					if (_container.size() == pos)
						pos--;

					// Get out of that container
					_container.pop();
				}

				// (2) This must be the entrance marker for a new container. Enter it
				else
				{
					// Push it
					_container.push(container_id);
				}
			}

		// Iterate over the container stack marking any _new_ entries as "visited"
		if (record_visits)
		{
			const container_t* con_iter;
			size_t num_new = _container.size() - pos;
			while (_container.iter(con_iter))
			{
				if (num_new <= 0)
					break;
				_globals->visit(*con_iter);
				--num_new;
			}
		}

		// Jump
		_ptr = dest;
	}
	template<frame_type type>
	void runner_impl::start_frame(uint32_t target) {
		if constexpr (type == frame_type::function) {
			// add a function start marker
			_output << values::func_start;
		}
		// Push next address onto the callstack
		{
		size_t address = _ptr - _story->instructions();
			_stack.push_frame<type>(address, bEvaluationMode);
			_ref_stack.push_frame<type>(address, bEvaluationMode);
		}
		bEvaluationMode = false; // unset eval mode when enter function or tunnel

		// Do the jump
		inkAssert(_story->instructions() + target < _story->end(), "Diverting past end of story data!");
		jump(_story->instructions() + target);
	}

	frame_type runner_impl::execute_return()
	{
		// Pop the callstack
		_ref_stack.fetch_values(_stack);
		frame_type type;
		offset_t offset = _stack.pop_frame(&type,bEvaluationMode);
		_ref_stack.push_values(_stack);
		{ 	frame_type t; bool eval;
			// TODO: write all refs to new frame 
			offset_t o = _ref_stack.pop_frame(&t, eval);
			inkAssert(o == offset && t == type && eval == bEvaluationMode,
					"_ref_stack and _stack should be in frame sync!");
		}

		// SPECIAL: On function, do a trim
		if (type == frame_type::function)
			_output << values::func_end;
		else if(type == frame_type::tunnel) {
			// if we return but there is a divert target on top of 
			// the evaluation stack, we should follow this instead
			// inkproof: I060
			if(!_eval.is_empty() && _eval.top().type() == value_type::divert) {
				start_frame<frame_type::tunnel>(_eval.pop().get<value_type::divert>());
				return type;
			}
		}


		// Jump to the old offset
		inkAssert(_story->instructions() + offset < _story->end(), "Callstack return is outside bounds of story!");
		jump(_story->instructions() + offset, false);

		// Return frame type
		return type;
	}

	runner_impl::runner_impl(const story_impl* data, globals global)
		: _story(data), _globals(global.cast<globals_impl>()),
		_operations(
				global.cast<globals_impl>()->strings(),
				global.cast<globals_impl>()->lists(),
				_rng,
				*global.cast<globals_impl>(),
				*data,
				static_cast<const runner_interface&>(*this)),
		_backup(nullptr), _done(nullptr), _choices(), _container(~0)
	{
		_ptr = _story->instructions();
		bEvaluationMode = false;

		// register with globals
		_globals->add_runner(this);
		if(_globals->lists()) {
			_output.set_list_meta(_globals->lists());
		}

		// initialize globals if necessary
		if (!_globals->are_globals_initialized())
		{
			_globals->initialize_globals(this);

			// Set us back to the beginning of the story
			reset();
			_ptr = _story->instructions();
		}
	}

	runner_impl::~runner_impl()
	{
		// unregister with globals
		_globals->remove_runner(this);
	}

#ifdef INK_ENABLE_STL
	std::string runner_impl::getline()
	{
		std::string result{""};
		bool fill = false;
		do {
			if (fill) {
				result += " ";
			}
			// Advance interpreter one line
			advance_line();
			// Read line into std::string
			result += _output.get();
			fill = _output.last_char() == ' ';
		} while(_ptr != nullptr && _output.last_char() != '\n');

		// TODO: fallback choice = no choice
		if(!has_choices() && _fallback_choice) { choose(~0); }

		// Return result
		inkAssert(_output.is_empty(), "Output should be empty after getline!");
		return result;
	}

	void runner_impl::getline(std::ostream& out)
	{
		bool fill = false;
		do {
			if (fill) { out << " "; }
			// Advance interpreter one line
			advance_line();
			// Write into out
			out << _output;
			fill = _output.last_char() == ' ';
		} while(_ptr != nullptr && _output.last_char() != '\n');

		// TODO: fallback choice = no choice
		if(!has_choices() && _fallback_choice) { choose(~0); }

		// Make sure we read everything
		inkAssert(_output.is_empty(), "Output should be empty after getline!");
	}

	std::string runner_impl::getall()
	{
		// Advance interpreter until we're stopped
		std::stringstream str;
		while(can_continue()) {
			getline(str);
		}

		// Read output into std::string

		// Return result
		inkAssert(_output.is_empty(), "Output should be empty after getall!");
		return str.str();
	}

	void runner_impl::getall(std::ostream& out)
	{
		// Advance interpreter until we're stopped
		while (can_continue())
			advance_line();

		// Send output into stream
		out << _output;

		// Return result
		inkAssert(_output.is_empty(), "Output should be empty after getall!");
	}

#endif
#ifdef INK_ENABLE_UNREAL
	FString runner_impl::getline()
	{
		clear_tags();
		FString result{};
		bool fill = false;
		do {
			if ( fill ) {
				result += " ";
			}
			// Advance interpreter one line
			advance_line();
			// Read lin ve into std::string
			const char* str = _output.get_alloc(_globals->strings(), _globals->lists());
			result.Append( str, c_str_len( str ) );
			fill = _output.last_char() == ' ';
		} while ( _ptr != nullptr && _output.last_char() != '\n' );

		// TODO: fallback choice = no choice
		if ( !has_choices() && _fallback_choice ) { choose( ~0 ); }

		// Return result
		inkAssert( _output.is_empty(), "Output should be empty after getline!" );
		return result;
	}
#endif

	void runner_impl::advance_line()
	{
		// Step while we still have instructions to execute
		while (_ptr != nullptr)
		{
			// Stop if we hit a new line
			if (line_step())
				break;
		}

		// can be in save state becaues of choice
		// Garbage collection TODO: How often do we want to do this?
		_globals->gc();
	}

	bool runner_impl::can_continue() const
	{
		return _ptr != nullptr;
	}

	void runner_impl::choose(size_t index)
	{
		if(has_choices()) {
			inkAssert(index < _choices.size(), "Choice index out of range");
		}
		restore(); // restore to stack state when choice was maked
		_globals->turn();
		// Get the choice
		const auto& c = has_choices() ? _choices[index] : _fallback_choice.value();

		// Get its thread
		thread_t choiceThread = c._thread;

		// Figure out where our previous pointer was for that thread
		ip_t prev = nullptr;
		if (choiceThread == ~0)
			prev = _done;
		else
			prev = _threads.get(choiceThread);

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
		jump(_story->instructions() + c.path(), false);
		if(!_container.empty()){ _globals->visit(_container.top()); }
		clear_choices();
		clear_tags();
	}

	void runner_impl::getline_silent()
	{
		// advance and clear output stream
		advance_line();
		_output.clear();
	}

	bool runner_impl::has_tags() const
	{
		return _tags.size() > 0;
	}

	size_t runner_impl::num_tags() const
	{
		return _tags.size();
	}

	const char* runner_impl::get_tag(size_t index) const
	{
		inkAssert(index < _tags.size(), "Tag index exceeds _num_tags");
		return _tags[index];
	}

#ifdef INK_ENABLE_CSTD
	char* runner_impl::getline_alloc()
	{                                         
		/// TODO
		inkFail("Not implemented yet!");
		return nullptr;
	}
#endif

	bool runner_impl::move_to(hash_t path)
	{
		// find the path
		ip_t destination = _story->find_offset_for(path);
		if (destination == nullptr)
		{
			// TODO: Error state?
			return false;
		}
		
		// Clear state and move to destination
		reset();
		_ptr = _story->instructions();
		jump(destination);

		return true;
	}

	void runner_impl::internal_bind(hash_t name, internal::function_base* function)
	{
		_functions.add(name, function);
	}

	runner_impl::change_type runner_impl::detect_change() const
	{
		// Check if the old newline is still present (hasn't been glu'd) and
		//  if there is new text (non-whitespace) in the stream since saving
		bool stillHasNewline = _output.saved_ends_with(value_type::newline);
		bool hasAddedNewText = _output.text_past_save();

		// Newline is still there and there's no new text
		if (stillHasNewline && !hasAddedNewText)
			return change_type::no_change;

		// If the newline is gone, we got glue'd. Continue as if we never had that newline
		if (!stillHasNewline)
			return change_type::newline_removed;

		// TODO New Tags -> extended

		// If there's new text content, we went too far
		if (hasAddedNewText)
			return change_type::extended_past_newline;

		inkFail("Invalid change detction. Never should be here!");
		return change_type::no_change;
	}

	bool runner_impl::line_step()
	{
		// Step the interpreter
		step();

		// If we're not within string evaluation
		if (!_output.has_marker())
		{
			// If we have a saved state after a previous newline
			// don't do this if we behind choice 
			if (_saved && !has_choices() && !_fallback_choice)
			{
				// Check for changes in the output stream
				switch (detect_change())
				{
				case change_type::extended_past_newline:
					// We've gone too far. Restore to before we moved past the newline and return that we are done
					restore();
					return true;
				case change_type::newline_removed:
					// Newline was removed. Proceed as if we never hit it
					forget();
					break;
				case change_type::no_change:
					break;
				}
			}

			// If we're on a newline
			if (_output.ends_with(value_type::newline))
			{
				// TODO: REMOVE
				// return true;

				// Unless we are out of content, we are going to try
				//  to continue a little further. This is to check for
				//  glue (which means there is potentially more content
				//  in this line) OR for non-text content such as choices.
				if (_ptr != nullptr)
				{
					// Save a snapshot of the current runtime state so we
					//  can return here if we end up hitting a new line
					forget();
					save();
				}
				// Otherwise, make sure we don't have any snapshots hanging around
				// expect we are in choice handleing
				else if( !has_choices() && !_fallback_choice)  {
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
#ifndef INK_ENABLE_UNREAL
		try
#endif
		{
			inkAssert(_ptr != nullptr, "Can not step! Do not have a valid pointer");

			// Load current command
			Command cmd = read<Command>();
			CommandFlag flag = read<CommandFlag>();

			// If we're falling and we hit a non-fallthrough command, stop the fall.
			if (_is_falling && !((cmd == Command::DIVERT && flag & CommandFlag::DIVERT_IS_FALLTHROUGH) || cmd == Command::END_CONTAINER_MARKER))
			{
				_is_falling = false;
				set_done_ptr(nullptr);
			}
			if (cmd >= Command::OP_BEGIN && cmd < Command::OP_END)
			{
				_operations(cmd, _eval);
			}
			else switch (cmd)
			{
				// == Value Commands ==
			case Command::STR:
			{
				const char* str = read<const char*>();
				if (bEvaluationMode)
					_eval.push(value{}.set<value_type::string>(str));
				else
					_output << value{}.set<value_type::string>(str);
			}
			break;
			case Command::INT:
			{
				int val = read<int>();
				if (bEvaluationMode)
					_eval.push(value{}.set<value_type::int32>(val));
				// TEST-CASE B006 don't print integers
			}
			break;
			case Command::BOOL:
			{
				bool val = read<int>() ? true : false;
				if(bEvaluationMode)
					_eval.push(value{}.set<value_type::boolean>(val));
				else
					_output << value{}.set<value_type::boolean>(val);
			}
			break;
			case Command::FLOAT:
			{
				float val = read<float>();
				if (bEvaluationMode)
					_eval.push(value{}.set<value_type::float32>(val));
				// TEST-CASE B006 don't print floats
			} break;
			case Command::VALUE_POINTER:
			{
				hash_t val = read<hash_t>();
				if(bEvaluationMode) {
					_eval.push(value{}.set<value_type::value_pointer>(val, static_cast<char>(flag) - 1));
				} else {
					inkFail("never conciderd what should happend here! (value pointer print)");
				}
			}
			break;
			case Command::LIST:
			{
				list_table::list list(read<int>());
				if(bEvaluationMode)
					_eval.push(value{}.set<value_type::list>(list));
				else {
					char* str = _globals->strings().create(_globals->lists().stringLen(
								list)+1);
					_globals->lists().toString(str, list)[0] = 0;
					_output << value{}.set<value_type::string>(str);
				}
			}
			break;
			case Command::DIVERT_VAL:
			{
				inkAssert(bEvaluationMode, "Can not push divert value into the output stream!");

				// Push the divert target onto the stack
				uint32_t target = read<uint32_t>();
				_eval.push(value{}.set<value_type::divert>(target));
			}
			break;
			case Command::NEWLINE:
			{
				if (bEvaluationMode)
					_eval.push(values::newline);
				else
					_output << values::newline;
			}
			break;
			case Command::GLUE:
			{
				if (bEvaluationMode)
					_eval.push(values::glue);
				else
					_output << values::glue;
			}
			break;
			case Command::VOID:
			{
				if (bEvaluationMode)
					_eval.push(values::null); // TODO: void type?
			}
			break;

			// == Divert commands
			case Command::DIVERT:
			{
				// Find divert address
				uint32_t target = read<uint32_t>();

				// Check for condition
				if (flag & CommandFlag::DIVERT_HAS_CONDITION && !_eval.pop().get<value_type::boolean>())
					break;

				// SPECIAL: Fallthrough divert. We're starting to fall out of containers
				if (flag & CommandFlag::DIVERT_IS_FALLTHROUGH && !_is_falling)
				{
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
					if (_stack.has_frame(&type) && type == frame_type::function) // implicit return is only for functions
					{
						// push null and return
						_eval.push(values::null);

						// HACK
						_ptr += sizeof(Command) + sizeof(CommandFlag);
						execute_return();
					}
					else 
					{
						on_done(false);
					}
					break;
				}

				// Do the jump
				inkAssert(_story->instructions() + target < _story->end(), "Diverting past end of story data!");
				jump(_story->instructions() + target);
			}
			break;
			case Command::DIVERT_TO_VARIABLE:
			{
				// Get variable value
				hash_t variable = read<hash_t>();

				// Check for condition
				if (flag & CommandFlag::DIVERT_HAS_CONDITION && !_eval.pop().get<value_type::boolean>())
					break;

				const value* val = get_var(variable);
				inkAssert(val, "Jump destiniation needs to be defined!");

				// Move to location
				jump(_story->instructions() + val->get<value_type::divert>());
				inkAssert(_ptr < _story->end(), "Diverted past end of story data!");
			}
			break;

			// == Terminal commands
			case Command::DONE:
				on_done(true);
				break;

			case Command::END:
				_ptr = nullptr;
				break;

			// == Tunneling
			case Command::TUNNEL:
			{
				uint32_t target;
				// Find divert address
				if(flag & CommandFlag::TUNNEL_TO_VARIABLE) {
					hash_t var_name = read<hash_t>();
					const value* val = get_var(var_name);
					inkAssert(val != nullptr, "Variable containing tunnel target could not be found!");
					target = val->get<value_type::divert>();
				} else {
					target = read<uint32_t>();
				}
				start_frame<frame_type::tunnel>(target);
			}
			break;
			case Command::FUNCTION:
			{
				uint32_t target;
				// Find divert address
				if(flag & CommandFlag::FUNCTION_TO_VARIABLE) {
					hash_t var_name = read<hash_t>();
					const value* val = get_var(var_name);
					inkAssert(val != nullptr, "Varibale containing function could not be found!");
					target  = val->get<value_type::divert>();
				} else {
					target = read<uint32_t>();
				}
				start_frame<frame_type::function>(target);
			}
			break;
			case Command::TUNNEL_RETURN:
			case Command::FUNCTION_RETURN:
			{
				execute_return();
			}
			break;

			case Command::THREAD:
			{
				// Push a thread frame so we can return easily
				// TODO We push ahead of a single divert. Is that correct in all cases....?????
				auto returnTo = _ptr + CommandSize<uint32_t>;
				_stack.push_frame<frame_type::thread>(returnTo - _story->instructions(), bEvaluationMode);
				_ref_stack.push_frame<frame_type::thread>(returnTo - _story->instructions(), bEvaluationMode);

				// Fork a new thread on the callstack
				thread_t thread = _stack.fork_thread();
				{
					thread_t t = _ref_stack.fork_thread();
					inkAssert(t == thread, "ref_stack and stack should be in sync!");
				}

				// Push that thread onto our thread stack
				_threads.push(thread);
			}
			break;

			// == set temporärie variable
			case Command::DEFINE_TEMP:
			{
				hash_t variableName = read<hash_t>();
				bool is_redef = flag & CommandFlag::ASSIGNMENT_IS_REDEFINE;

				// Get the top value and put it into the variable
				value v = _eval.pop();
				set_var<Scope::LOCAL>(variableName, v, is_redef);
			}
			break;

			case Command::SET_VARIABLE:
			{
				hash_t variableName = read<hash_t>();

				// Check if it's a redefinition (not yet used, seems important for pointers later?)
				bool is_redef = flag & CommandFlag::ASSIGNMENT_IS_REDEFINE;

				// If not, we're setting a global (temporary variables are explicitely defined as such,
				//  where globals are defined using SET_VARIABLE).
				value val = _eval.pop();
				if(is_redef) {
					set_var(variableName, val, is_redef);
				} else {
					set_var<Scope::GLOBAL>(variableName, val, is_redef);
				}
			}
			break;

			// == Function calls
			case Command::CALL_EXTERNAL:
			{
				// Read function name
				hash_t functionName = read<hash_t>();

				// Interpret flag as argument count
				int numArguments = (int)flag;

				// find and execute. will automatically push a valid if applicable
				bool success = _functions.call(functionName, &_eval, numArguments, _globals->strings());

				// If we failed, we need to at least pretend so our state doesn't get fucked
				if (!success)
				{
					// pop arguments
					for (int i = 0; i < numArguments; i++)
						_eval.pop();

					// push void
					_eval.push(value());
				}

				// TODO: Verify something was found?
			}
			break;

			// == Evaluation stack
			case Command::START_EVAL:
				bEvaluationMode = true;
				break;
			case Command::END_EVAL:
				bEvaluationMode = false;

				// Assert stack is empty? Is that necessary?
				break;
			case Command::OUTPUT:
			{
				value v = _eval.pop();
				_output << v;
			}
			break;
			case Command::POP:
				_eval.pop();
				break;
			case Command::DUPLICATE:
				_eval.push(_eval.top_value());
				break;
			case Command::PUSH_VARIABLE_VALUE:
			{
				// Try to find in local stack
				hash_t variableName = read<hash_t>();
				const value* val = get_var(variableName);

				inkAssert(val != nullptr, "Could not find variable!");
				_eval.push(*val);
				break;
			}
			case Command::START_STR:
			{
				inkAssert(bEvaluationMode, "Can not enter string mode while not in evaluation mode!");
				bEvaluationMode = false;
				_output << values::marker;
			} break;
			case Command::END_STR:
			{
				// TODO: Assert we really had a marker on there?
				inkAssert(!bEvaluationMode, "Must be in evaluation mode");
				bEvaluationMode = true;

				// Load value from output stream
				// Push onto stack
				_eval.push(value{}.set<value_type::string>(_output.get_alloc<false>(
								_globals->strings(),
								_globals->lists())));
			} break;

			// == Choice commands
			case Command::CHOICE:
			{
				// Read path
				uint32_t path = read<uint32_t>();

				// If we're a once only choice, make sure our destination hasn't
				//  been visited
				if (flag & CommandFlag::CHOICE_IS_ONCE_ONLY) {
					// Need to convert offset to container index
					container_t destination = -1;
					if (_story->get_container_id(_story->instructions() + path, destination))
					{
						// Ignore the choice if we've visited the destination before
						if (_globals->visits(destination) > 0)
							break;
					}
					else
					{
						inkAssert(false, "Destination for choice block does not have counting flags.");
					}
				}

				// Choice is conditional
				if (flag & CommandFlag::CHOICE_HAS_CONDITION) {
					// Only show if the top of the eval stack is 'truthy'
					if (!_eval.pop().get<value_type::boolean>())
						break;
				}

				// Use a marker to start compiling the choice text
				_output << values::marker;
				value stack[2];
				int sc = 0;

				if (flag & CommandFlag::CHOICE_HAS_START_CONTENT) {
					stack[sc++] = _eval.pop();
				}
				if (flag & CommandFlag::CHOICE_HAS_CHOICE_ONLY_CONTENT) {
					stack[sc++] = _eval.pop();
				}
				for(;sc;--sc) { _output << stack[sc-1]; }

				// Create choice and record it
				if (flag & CommandFlag::CHOICE_IS_INVISIBLE_DEFAULT) {
					_fallback_choice
						= choice{}.setup(_output, _globals->strings(), _globals->lists(), _choices.size(), path, current_thread());
				} else {
					add_choice().setup(_output, _globals->strings(), _globals->lists(), _choices.size(), path, current_thread());
				}
				// save stack at last choice
				if(_saved) { forget(); }
				save();
			} break;
			case Command::START_CONTAINER_MARKER:
			{
				// Keep track of current container
				_container.push(read<uint32_t>());

				// Increment visit count
				if (flag & CommandFlag::CONTAINER_MARKER_TRACK_VISITS)
				{
					_globals->visit(_container.top());
				}

				// TODO Turn counts
			} break;
			case Command::END_CONTAINER_MARKER:
			{
				container_t index = read<container_t>();
				inkAssert(_container.top() == index, "Leaving container we are not in!");

				// Move up out of the current container
				_container.pop();

				// SPECIAL: If we've popped all containers, then there's an implied 'done' command or return
				if (_container.empty())
				{
					_is_falling = false;

					frame_type type;
					if (!_threads.empty())
					{
						on_done(false);
						return;
					}
					else if (_stack.has_frame(&type) && type == frame_type::function) // implicit return is only for functions
					{
						// push null and return
						_eval.push(values::null);

						// HACK
						_ptr += sizeof(Command) + sizeof(CommandFlag);
						execute_return();
					}
					/*else TODO I had to remove this to make a test work.... is this important? Have I broken something?
					{
						on_done(false); // do we need to not set _done here? It wasn't set in the original code #implieddone
						return;
					}*/
				}
			} break;
			case Command::VISIT:
			{
				// Push the visit count for the current container to the top
				//  is 0-indexed for some reason. idk why but this is what ink expects
				_eval.push(value{}.set<value_type::int32>((int)_globals->visits(_container.top()) - 1));
			} break;
			case Command::SEQUENCE:
			{
				// TODO: The C# ink runtime does a bunch of fancy logic
				//  to make sure each element is picked at least once in every
				//  iteration loop. I don't feel like replicating that right now.
				// So, let's just return a random number and *shrug*
				int sequenceLength = _eval.pop().get<value_type::int32>();
				int index = _eval.pop().get<value_type::int32>();

				_eval.push(value{}.set<value_type::int32>(_rng.rand(sequenceLength)));
			} break;
			case Command::SEED:
			{
				int32_t seed = _eval.pop().get<value_type::int32>();
				_rng.srand(seed);

				_eval.push(values::null);
			} break;

			case Command::READ_COUNT:
			{
				// Get container index
				container_t container = read<container_t>();

				// Push the read count for the requested container index
				_eval.push(value{}.set<value_type::int32>((int)_globals->visits(container)));
			} break;
			case Command::TAG:
			{
				_tags.push() = read<const char*>();
			} break;
			default:
				inkAssert(false, "Unrecognized command!");
				break;
			}
		}
#ifndef INK_ENABLE_UNREAL
		catch (...)
		{
			// Reset our whole state as it's probably corrupt
			reset();
			throw;
		}
#endif
	}

	void runner_impl::on_done(bool setDone)
	{
		// If we're in a thread
		if (!_threads.empty())
		{
			// Get the thread ID of the current thread
			thread_t completedThreadId = _threads.pop();

			// Push in a complete marker
			_stack.complete_thread(completedThreadId);
			_ref_stack.complete_thread(completedThreadId);

			// Go to where the thread started
			frame_type type = execute_return();
			inkAssert(type == frame_type::thread, "Expected thread frame marker to hold return to value but none found...");
			// if thread ends, move stave point with, else the thread end marker is missing
			// and we can't collect the other threads
			if(_saved) { forget(); save(); }
		}
		else
		{
			if (setDone)
				set_done_ptr(_ptr);
			_ptr = nullptr;
		}
	}

	void runner_impl::set_done_ptr(ip_t ptr)
	{
		thread_t curr = current_thread();
		if (curr == ~0) {
			_done = ptr;
		}
		else {
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
		bEvaluationMode = false;
		_saved = false;
		_choices.clear();
		_ptr = nullptr;
		_done = nullptr;
		_container.clear();
	}

	void runner_impl::mark_strings(string_table& strings) const
	{
		// Find strings in output and stacks
		_output.mark_strings(strings);
		_stack.mark_strings(strings);
		// ref_stack has no strings!
		_eval.mark_strings(strings);

		// Take into account choice text
		for (size_t i = 0; i < _choices.size(); i++)
			strings.mark_used(_choices[i]._text);
	}

	void runner_impl::save()
	{
		inkAssert(!_saved, "Runner state already saved");

		_saved = true;
		_output.save();
		_stack.save();
		_ref_stack.save();
		_backup = _ptr;
		_container.save();
		_globals->save();
		_eval.save();
		_threads.save();
		_backup_choice_len = _choices.size();
		bSavedEvaluationMode = bEvaluationMode;

		// Not doing this anymore. There can be lingering stack entries from function returns
		// inkAssert(_eval.is_empty(), "Can not save interpreter state while eval stack is not empty");
	}

	void runner_impl::restore()
	{
		inkAssert(_saved, "Can't restore. No runner state saved.");
		// the output can be restored without the rest
		if(_output.saved()) {_output.restore(); }
		_stack.restore();
		_ref_stack.restore();
		_ptr = _backup;
		_container.restore();
		_globals->restore();
		_eval.restore();
		_threads.restore();
		_choices.resize(_backup_choice_len);
		bEvaluationMode = bSavedEvaluationMode;

		// Not doing this anymore. There can be lingering stack entries from function returns
		// inkAssert(_eval.is_empty(), "Can not save interpreter state while eval stack is not empty");

		_saved = false;
	}

	void runner_impl::forget()
	{
		// Do nothing if we haven't saved
		if (!_saved)
			return;

		_output.forget();
		_stack.forget();
		_ref_stack.forget();
		_container.forget();
		_globals->forget();
		_eval.forget();
		_threads.forget();

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
}
