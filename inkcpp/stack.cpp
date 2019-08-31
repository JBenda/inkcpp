#include "stack.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			basic_stack::basic_stack(entry* data, size_t size)
				: _stack(data), _size(size), _pos(0), _save(~0)
			{

			}

			void basic_stack::set(hash_t name, const value& val)
			{
				// If we have a save point, always add no matter what
				if (_save != ~0)
				{
					add(name, val);
					return;
				}

				// Either set an existing variable or add it to the stack
				value* existing = const_cast<value*>(get(name));
				if (existing == nullptr)
					add(name, val);
				else
					*existing = val;
			}

			const value* basic_stack::get(hash_t name) const
			{
				if (_pos == 0)
					return nullptr;

				// Move backwards and find the variable
				size_t i = _pos;
				do
				{
					i--;

					if (_stack[i].name == name)
						return &_stack[i].data;

					// We hit the top of this stack frame. Not found!
					if (_stack[i].name == InvalidHash)
						break;

					// Jump over saved data
					if (i == _save)
						i = _jump;
				} while (i > 0);

				return nullptr;
			}

			void basic_stack::push_frame(offset_t return_to)
			{
				// Add to top of stack
				add(InvalidHash, return_to);
			}

			offset_t basic_stack::pop_frame()
			{
				assert(_pos > 0, "Can not pop frame from empty callstack");

				// Advance up the callstack until we find the frame record
				_pos--;
				while (_stack[_pos].name != InvalidHash && _pos > 0)
				{
					// Jump over saved data
					if (_pos == _save)
						_pos = _jump;

					_pos--;
				}

				assert(_stack[_pos].name == InvalidHash, "Attempting to pop_frame when no frames exist! Stack reset");

				// Return the offset stored in the frame record
				return _stack[_pos].data.as_divert();
			}

			void basic_stack::save()
			{
				assert(_save == ~0, "Can not save stack twice! restore() or forget() first");

				// Save current stack position
				_save = _jump = _pos;
			}

			void basic_stack::restore()
			{
				assert(_save != ~0, "Can not restore() when there is no save!");

				// Move position back to saved position
				_pos = _save;
				_save = _jump = ~0;
			}

			void basic_stack::forget()
			{
				assert(_save != ~0, "Can not forget when the stack has never been saved!");

				// If we have moven to a point earlier than the save point but we have a jump point
				if (_pos < _save && _pos > _jump)
				{
					// Everything between the jump point and the save point needs to be nullified
					for (size_t i = _jump; i < _save; i++)
						_stack[i].name = ~0;
				}

				// Just reset save position
				_save = ~0;
			}

			void basic_stack::add(hash_t name, const value& val)
			{
				// Don't destroy saved data
				if (_pos < _save && _save != ~0)
				{
					// Move to next safe spot after saved data and save where we came from
					_jump = _pos;
					_pos = _save;
				}

				assert(_pos < _size, "Stack overflow!");

				// Push onto the top of the stack
				_stack[_pos].name = name;
				_stack[_pos].data = val;
				_pos++;
			}

			basic_eval_stack::basic_eval_stack(value* data, size_t size)
				: _stack(data), _size(size), _pos(0)
			{

			}

			void basic_eval_stack::push(const value& val)
			{
				assert(_pos < _size, "Stack overflow!");
				_stack[_pos++] = val;
			}

			value basic_eval_stack::pop()
			{
				assert(_pos > 0, "Nothing left to pop!");

				// Decrement and return
				_pos--;
				return _stack[_pos];
			}

			bool basic_eval_stack::is_empty() const
			{
				return _pos == 0;
			}

			void basic_eval_stack::clear()
			{
				_pos = 0;
			}

		}
	}
}