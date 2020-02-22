#include "runner_impl.h"
#include "story_impl.h"
#include "command.h"
#include "choice.h"
#include "globals_impl.h"

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
	template<typename T>
	inline T runner_impl::read()
	{
		// Sanity
		inkAssert(_ptr + sizeof(T) <= _story->end(), "Unexpected EOF in Ink execution");

		// Read memory
		T val = *(const T*)_ptr;

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
		inkAssert(_num_choices < MAX_CHOICES, "Ran out of choice storage!");
		return _choices[_num_choices++];
	}

	void runner_impl::clear_choices()
	{
		// TODO: Garbage collection?
		_num_choices = 0;
	}

	void runner_impl::jump(ip_t dest, bool record_visits)
	{
		// Optimization: if we are _is_falling, then we can
		//  _should be_ able to safely assume that there is nothing to do here. A falling
		//  divert should only be taking us from a container to that same container's end point
		//  without entering any other containers
		if (_is_falling)
		{
			_ptr = dest;
			return;
		}

		// Check which direction we are jumping
		bool reverse = dest < _ptr;

		// iteration
		const uint32_t* iter = nullptr;
		container_t container_id;
		ip_t offset;

		// Iterate until we find the container marker just before our own
		while (_story->iterate_containers(iter, container_id, offset, reverse)) {
			if (!reverse && offset > _ptr || reverse && offset < _ptr) {

				// Step back once in the iteration and break
				_story->iterate_containers(iter, container_id, offset, !reverse);
				break;
			}
		}

		size_t pos = _container.size();

		// Start moving forward (or backwards)
		while (_story->iterate_containers(iter, container_id, offset, reverse))
		{
			// Break when we've past the destination
			if (!reverse && offset > dest || reverse && offset <= dest)
				break;

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
			const container_t* iter;
			size_t num_new = _container.size() - pos;
			while (_container.iter(iter))
			{
				if (num_new == 0)
					break;
				_globals->visit(*iter);
				num_new--;
			}
		}

		// Jump
		_ptr = dest;
	}

	void runner_impl::run_binary_operator(unsigned char cmd)
	{
		// Pop
		value rhs = _eval.pop(), lhs = _eval.pop();
		value result;

		switch ((Command)cmd)
		{
		case Command::ADD:
			result = value::add(lhs, rhs, _output, _strings);
			break;
		case Command::SUBTRACT:
			result = lhs - rhs;
			break;
		case Command::DIVIDE:
			result = lhs / rhs;
			break;
		case Command::MULTIPLY:
			result = lhs * rhs;
			break;
		case Command::MOD:
			result = lhs % rhs;
			break;
		case Command::IS_EQUAL:
			result = lhs == rhs;
			break;
		case Command::GREATER_THAN:
			result = lhs > rhs;
			break;
		case Command::LESS_THAN:
			result = lhs < rhs;
			break;
		case Command::GREATER_THAN_EQUALS:
			result = lhs >= rhs;
			break;
		case Command::LESS_THAN_EQUALS:
			result = lhs <= rhs;
			break;
		case Command::NOT_EQUAL:
			result = lhs != rhs;
			break;
		case Command::AND:
			result = lhs && rhs;
			break;
		case Command::OR:
			result = lhs || rhs;
			break;
		case Command::MIN:
			result = lhs < rhs ? lhs : rhs;
			break;
		case Command::MAX:
			result = lhs > rhs ? lhs : rhs;
			break;
		}

		// Push result onto the stack
		_eval.push(result);
	}

	void runner_impl::run_unary_operator(unsigned char cmd)
	{
		// Pop
		value v = _eval.pop();
		value result;

		// Run command
		switch ((Command)cmd)
		{
		case Command::NEGATE:
			result = -v;
			break;
		case Command::NOT:
			result = !v;
			break;
		}

		// Push to the stack
		_eval.push(result);
	}

	runner_impl::runner_impl(const story_impl* data, globals global)
		: _story(data), _globals(global.cast<globals_impl>()), _container(~0)
	{
		_ptr = _story->instructions();
		bEvaluationMode = false;
	}

#ifdef INK_ENABLE_STL
	std::string runner_impl::getline()
	{
		// Advance interpreter one line
		advance_line();

		// Read line into std::string
		std::string result;
		_output >> result;

		// Return result
		inkAssert(_output.is_empty(), "Output should be empty after getline!");
		return result;
	}

	void runner_impl::getline(std::ostream& out)
	{
		// Advance interpreter one line
		advance_line();

		// Write into out
		out << _output;

		// Make sure we read everything
		inkAssert(_output.is_empty(), "Output should be empty after getline!");
	}

#endif
#ifdef INK_ENABLE_UNREAL
	FString runner_impl::getline()
	{
		// Advance interpreter one line
		advance_line();

		// Read line into std::string
		FString result;
		_output >> result;

		// Return result
		inkAssert(_output.is_empty(), "Output should be empty after getline!");
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

		// Eval stack should be cleared
		// (this can have lingering elements if, for example, a choice is conditional
		//   and doesn't end up popping its text/etc. off the eval stack)
		_eval.clear();

		// Should be nothing in the eval stack
		// inkAssert(_eval.is_empty(), "Eval stack should be empty after advancing one line");
		inkAssert(!_saved, "Should be no state snapshot at the end of newline");

		// Garbage collection TODO: How often do we want to do this?
		gc();
	}

	bool runner_impl::can_continue() const
	{
		return _ptr != nullptr;
	}

	void runner_impl::choose(size_t index)
	{
		// Restore pointer to the last "done" point
		inkAssert(_done != nullptr, "No 'done' point recorded before finishing choice output");
		inkAssert(index < _num_choices, "Choice index out of range");
		jump(_done, false);
		_done = nullptr;

		// Jump to destination and clear choice list
		jump(_story->instructions() + _choices[index].path());
		clear_choices();
	}

#ifdef INK_ENABLE_CSTD
	char* runner_impl::getline_alloc()
	{
		// TODO
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
		bool stillHasNewline = _output.saved_ends_with(data_type::newline);
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

		// Nothing to report yet
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
			if (_saved)
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
				}
			}

			// If we're on a newline
			if (_output.ends_with(data_type::newline))
			{
				// TODO: REMOVE
				//return true;

				// Unless we are out of content, we are going to try
				//  to continue a little further. This is to check for
				//  glue (which means there is potentially more content
				//  in this line) OR for non-text content such as choices.
				if (_ptr != nullptr)
				{
					// Save a snapshot of the current runtime state so we
					//  can return here if we end up hitting a new line
					if (!_saved)
						save();
				}
				else // Otherwise, make sure we don't have any snapshots hanging around
					forget();
			}
		}

		return false;
	}

	void runner_impl::step()
	{
		try
		{
			inkAssert(_ptr != nullptr, "Can not step! Do not have a valid pointer");

			// Load current command
			Command cmd = read<Command>();
			CommandFlag flag = read<CommandFlag>();

			// If we're falling and we hit a non-fallthrough command, stop the fall.
			if (_is_falling && !((cmd == Command::DIVERT && flag & CommandFlag::DIVERT_IS_FALLTHROUGH) || cmd == Command::END_CONTAINER_MARKER))
			{
				_is_falling = false;
				_done = nullptr;
			}

			if (cmd >= Command::BINARY_OPERATORS_START && cmd <= Command::BINARY_OPERATORS_END)
			{
				run_binary_operator((unsigned char)cmd);
			}
			else if (cmd >= Command::UNARY_OPERATORS_START && cmd <= Command::UNARY_OPERATORS_END)
			{
				run_unary_operator((unsigned char)cmd);
			}
			else switch (cmd)
			{
				// == Value Commands ==
			case Command::STR:
			{
				const char* str = read<const char*>();
				if (bEvaluationMode)
					_eval.push(str);
				else
					_output << str;
			}
			break;
			case Command::INT:
			{
				int val = read<int>();
				if (bEvaluationMode)
					_eval.push(val);
				else
					_output << val;
			}
			break;

			case Command::DIVERT_VAL:
			{
				inkAssert(bEvaluationMode, "Can not push divert value into the output stream!");

				// Push the divert target onto the stack
				uint32_t target = read<uint32_t>();
				_eval.push(value(target));
			}
			break;
			case Command::NEWLINE:
			{
				if (bEvaluationMode)
					_eval.push(newline);
				else
					_output << newline;
			}
			break;
			case Command::GLUE:
			{
				if (bEvaluationMode)
					_eval.push(glue);
				else
					_output << glue;
			}
			break;

			// == Divert commands
			case Command::DIVERT:
			{
				// Find divert address
				uint32_t target = read<uint32_t>();

				// Check for condition
				if (flag & CommandFlag::DIVERT_HAS_CONDITION && !_eval.pop())
					break;

				// SPECIAL: Fallthrough divert. We're starting to fall out of containers
				if (flag & CommandFlag::DIVERT_IS_FALLTHROUGH && !_is_falling)
				{
					// Record the position of the instruction pointer at the first fallthrough.
					//  We'll use this if we run out of content and hit an implied "done" to restore
					//  our position when a choice is chosen. See ::choose
					_done = _ptr;
					_is_falling = true;
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
				if (flag & CommandFlag::DIVERT_HAS_CONDITION && !_eval.pop())
					break;

				const value& val = *_stack.get(variable);

				// Move to location
				jump(_story->instructions() + val.as_divert());
				inkAssert(_ptr < _story->end(), "Diverted past end of story data!");
			}
			break;

			// == Terminal commands
			case Command::DONE:
				_done = _ptr;
			case Command::END:
				_ptr = nullptr;
				break;

				// == Variable definitions
			case Command::DEFINE_TEMP:
			{
				hash_t variableName = read<hash_t>();

				// Get the top value and put it into the variable
				value v = _eval.pop();
				_stack.set(variableName, v);
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
				bool success = _functions.call(functionName, &_eval, numArguments);

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
				_eval.push(_eval.top());
				break;
			case Command::PUSH_VARIABLE_VALUE:
			{
				hash_t variableName = read<hash_t>();
				const value* val = _stack.get(variableName);
				inkAssert(val != nullptr, "Could not find temporary variable!");
				_eval.push(*val);
				break;
			}
			case Command::START_STR:
			{
				inkAssert(bEvaluationMode, "Can not enter string mode while not in evaluation mode!");
				bEvaluationMode = false;
				_output << marker;
			} break;
			case Command::END_STR:
			{
				// TODO: Assert we really had a marker on there?
				inkAssert(!bEvaluationMode, "Must be in evaluation mode");
				bEvaluationMode = true;

				// Load value from output stream
				value val;
				_output >> val;

				// Push onto stack
				_eval.push(val);
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
					if (!_eval.pop())
						break;
				}

				// Use a marker to start compiling the choice text
				_output << marker;

				if (flag & CommandFlag::CHOICE_HAS_START_CONTENT) {
					_output << _eval.pop();
				}
				if (flag & CommandFlag::CHOICE_HAS_CHOICE_ONLY_CONTENT) {
					_output << _eval.pop();
				}
				if (flag & CommandFlag::CHOICE_IS_INVISIBLE_DEFAULT) {} // TODO

				// Create choice and record it
				add_choice().setup(_output, _strings, _num_choices, path);
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

				// SPECIAL: If we've popped all containers, then there's an implied 'done' command
				if (_container.empty())
				{
					_is_falling = false;
					_ptr = nullptr;
					return;
				}
			} break;
			case Command::VISIT:
			{
				// Push the visit count for the current container to the top
				//  is 0-indexed for some reason. idk why but this is what ink expects
				_eval.push((int)_globals->visits(_container.top()) - 1);
			} break;
			case Command::SEQUENCE:
			{
				// TODO: The C# ink runtime does a bunch of fancy logic
				//  to make sure each element is picked at least once in every
				//  iteration loop. I don't feel like replicating that right now.
				// So, let's just return a random number and *shrug*
				int sequenceLength = _eval.pop();
				int index = _eval.pop();

				_eval.push(rand() % sequenceLength); // TODO: platform independance?
			} break;
			case Command::READ_COUNT:
			{
				// Get container index
				container_t container = read<container_t>();

				// Push the read count for the requested container index
				_eval.push((int)_globals->visits(container));
			} break;
			default:
				inkAssert(false, "Unrecognized command!");
				break;
			}
		}
		catch (...)
		{
			// Reset our whole state as it's probably corrupt
			reset();
			throw;
		}
	}

	void runner_impl::reset()
	{
		_eval.clear();
		_output.clear();
		_stack.clear();
		bEvaluationMode = false;
		_saved = false;
		_num_choices = 0;
		_ptr = nullptr;
		_done = nullptr;
		_container.clear();
	}

	void runner_impl::gc()
	{
		// Mark all strings as unused
		_strings.clear_usage();

		// Find strings
		_output.mark_strings(_strings);
		_stack.mark_strings(_strings);
		_eval.mark_strings(_strings);
		
		// Take into account choice text
		for (int i = 0; i < _num_choices; i++)
			_strings.mark_used(_choices[i]._text);

		// Clean up
		_strings.gc();
	}

	void runner_impl::save()
	{
		inkAssert(!_saved, "Runner state already saved");

		_saved = true;
		_output.save();
		_stack.save();
		_backup = _ptr;
		_container.save();
		_globals->save();

		// TODO: Save eval stack? Should be empty
		inkAssert(_eval.is_empty(), "Can not save interpreter state while eval stack is not empty");
	}

	void runner_impl::restore()
	{
		inkAssert(_saved, "Can't restore. No runner state saved.");

		_output.restore();
		_stack.restore();
		_ptr = _backup;
		_container.restore();
		_globals->restore();

		// TODO: Save eval stack? Should be empty
		inkAssert(_eval.is_empty(), "Can not save interpreter state while eval stack is not empty");

		_saved = false;
	}

	void runner_impl::forget()
	{
		// Do nothing if we haven't saved
		if (!_saved)
			return;

		_output.forget();
		_stack.forget();
		_container.forget();
		_globals->forget();

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