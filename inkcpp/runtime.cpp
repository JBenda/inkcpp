#include "runtime.h"
#include "command.h"
#include "story.h"
#include "choice.h"

namespace ink
{
	namespace runtime
	{
		using namespace internal;

		template<typename T>
		inline T runner::read()
		{
			// Sanity
			assert(_ptr + sizeof(T) <= _story->end(), "Unexpected EOF in Ink execution");

			// Read memory
			T val = *(const T*)_ptr;

			// Advance ip
			_ptr += sizeof(T);

			// Return
			return val;
		}

		template<>
		inline const char* runner::read()
		{
			offset_t str = read<offset_t>();
			return _story->string(str);
		}

		ink::runtime::choice& runner::add_choice()
		{
			assert(_num_choices < MAX_CHOICES, "Ran out of choice storage!");
			return _choices[_num_choices++];
		}

		void runner::clear_choices()
		{
			// TODO: Garbage collection?
			_num_choices = 0;
		}

		void runner::run_binary_operator(unsigned char cmd)
		{
			// Pop
			value rhs = _eval.pop(), lhs = _eval.pop();
			value result;

			switch ((Command)cmd)
			{
			case Command::ADD:
				result = lhs + rhs;
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

		void runner::run_unary_operator(unsigned char cmd)
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

		runner::runner(const story* data)
			: _story(data)
		{
			_ptr = _story->instructions();
			bEvaluationMode = false;
		}

#ifdef INK_ENABLE_STL
		std::string runner::getline()
		{
			// Advance interpreter one line
			advance_line();

			// Read line into std::string
			std::string result;
			_output >> result;

			// Return result
			assert(_output.is_empty(), "Output should be empty after getline!");
			return result;
		}

		void runner::getline(std::ostream& out)
		{
			// Advance interpreter one line
			advance_line();

			// Write into out
			out << _output;

			// Make sure we read everything
			assert(_output.is_empty(), "Output should be empty after getline!");
		}

#endif

		void runner::advance_line()
		{
			// Step while we still have instructions to execute
			while (_ptr != nullptr)
			{
				// Stop if we hit a new line
				if (line_step())
					break;
			}

			// Should be nothing in the eval stack
			assert(_eval.is_empty(), "Eval stack should be empty after advancing one line");
			assert(!_saved, "Should be no state snapshot at the end of newline");
		}

		bool runner::can_continue() const
		{
			return _ptr != nullptr;
		}

		const choice& runner::get_choice(size_t index)
		{
			assert(index < _num_choices, "Choice out of bounds!");
			return _choices[index];
		}

		void runner::choose(int index)
		{
			_ptr = _story->instructions() + _choices[index].path();
			clear_choices();
		}

		runner::change_type runner::detect_change() const
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

		bool runner::line_step()
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

		void runner::step()
		{
			try
			{
				assert(_ptr != nullptr, "Can not step! Do not have a valid pointer");

				// Load current command
				Command cmd = read<Command>();
				CommandFlag flag = read<CommandFlag>();

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
					assert(bEvaluationMode, "Can not push divert value into the output stream!");

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

					_ptr = _story->instructions() + target;
					assert(_ptr < _story->end(), "Diverted past end of story data!");
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
					_ptr = _story->instructions() + val.as_divert();
					assert(_ptr < _story->end(), "Diverted past end of story data!");
				}
				break;

				// == Terminal commands
				case Command::END:
				case Command::DONE:
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
					assert(val != nullptr, "Could not find temporary variable!");
					_eval.push(*val);
					break;
				}
				case Command::START_STR:
				{
					assert(bEvaluationMode, "Can not enter string mode while not in evaluation mode!");
					bEvaluationMode = false;
					_output << marker;
				} break;
				case Command::END_STR:
				{
					// TODO: Assert we really had a marker on there?
					assert(!bEvaluationMode);
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
					// Use a marker to start compiling the choice text
					_output << marker;

					if (flag & CommandFlag::CHOICE_HAS_CONDITION) {} // TODO
					if (flag & CommandFlag::CHOICE_HAS_START_CONTENT) {
						_output << _eval.pop();
					}
					if (flag & CommandFlag::CHOICE_HAS_CHOICE_ONLY_CONTENT) {
						_output << _eval.pop();
					}
					if (flag & CommandFlag::CHOICE_IS_INVISIBLE_DEFAULT) {} // TODO
					if (flag & CommandFlag::CHOICE_IS_ONCE_ONLY) {} // TODO

					// Read path
					uint32_t path = read<uint32_t>();

					// Create choice and record it
					add_choice().setup(_output, _num_choices, path);
				} break;
				}
			}
			catch(...)
			{
				// Reset our whole state as it's probably corrupt
				reset();
				throw;
			}
		}

		void runner::reset()
		{
			_eval.clear();
			_output.clear();
			_stack.clear();
			bEvaluationMode = false;
			_saved = false;
			_num_choices = 0;
			_ptr = nullptr;
		}

		void runner::save()
		{
			assert(!_saved, "Runner state already saved");

			_saved = true;
			_output.save();
			_stack.save();
			_backup = _ptr;

			// TODO: Save eval stack? Should be empty
			assert(_eval.is_empty(), "Can not save interpreter state while eval stack is not empty");
		}

		void runner::restore()
		{
			assert(_saved, "Can't restore. No runner state saved.");

			_output.restore();
			_stack.restore();
			_ptr = _backup;

			// TODO: Save eval stack? Should be empty
			assert(_eval.is_empty(), "Can not save interpreter state while eval stack is not empty");

			_saved = false;
		}

		void runner::forget()
		{
			// Do nothing if we haven't saved
			if (!_saved)
				return;

			_output.forget();
			_stack.forget();

			// Nothing to do for eval stack. It should just stay as it is

			_saved = false;
		}

#ifdef INK_ENABLE_STL
		std::ostream& operator<<(std::ostream& out, runner& in)
		{
			in.getline(out);
			return out;
		}
#endif
	}
}