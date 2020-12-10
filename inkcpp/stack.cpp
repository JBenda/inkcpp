#include "stack.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			basic_stack::basic_stack(entry* data, size_t size)
				: base(data, size)
			{
			}

			void basic_stack::set(hash_t name, const value& val)
			{
				// If we have a save point, always add no matter what
				if (base::is_saved())
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
				// Find whatever comes first: a matching entry or a stack frame entry
				const entry* found = base::reverse_find([name](entry& e) { return e.name == name || e.name == InvalidHash; });

				// If nothing found, no value
				if (found == nullptr)
					return nullptr;

				// If we found something of that name, return the value
				if (found->name == name)
					return &found->data;

				// Otherwise, nothing in this stack frame
				return nullptr;
			}

			void basic_stack::push_frame(offset_t return_to, frame_type type)
			{
				// Add to top of stack
				add(InvalidHash, value(return_to, type == frame_type::tunnel ? data_type::tunnel_frame : data_type::function_frame));
			}

			offset_t basic_stack::pop_frame(frame_type* type)
			{
				inkAssert(!base::is_empty(), "Can not pop frame from empty callstack.");

				const entry* popped = &base::pop([](const entry&) { return false; });
				while (popped->name != InvalidHash && !base::is_empty())
				{
					popped = &base::pop([](const entry&) { return false; });
				}

				inkAssert(popped->name == InvalidHash, "Attempting to pop_frame when no frames exist! Stack reset.");

				// Store frame type
				if (type != nullptr)
					*type = (popped->data.data_type() == data_type::tunnel_frame) ? frame_type::tunnel : frame_type::function;

				// Return the offset stored in the frame record
				return popped->data.as_divert();
			}

			bool basic_stack::has_frame() const
			{
				// Empty case
				if (base::is_empty())
					return false;

				// Search in reverse for a stack frame
				const entry* frame = base::reverse_find([](const entry& elem) {
					return elem.name == InvalidHash;
				});

				// Return true if a frame was found
				return frame != nullptr;
			}

			void basic_stack::clear()
			{
				base::clear();
			}

			void basic_stack::mark_strings(string_table& strings) const
			{
				// Mark all strings
				base::for_each_all([&strings](const entry& elem) { elem.data.mark_strings(strings); });
			}

			void basic_stack::save()
			{
				base::save();
			}

			void basic_stack::restore()
			{
				base::restore();
			}

			void basic_stack::forget()
			{
				base::forget([](entry& elem) { elem.name = ~0; });
			}

			void basic_stack::add(hash_t name, const value& val)
			{
				base::push({ name, val });
			}

			basic_eval_stack::basic_eval_stack(value* data, size_t size)
				: base(data, size)
			{

			}

			void basic_eval_stack::push(const value& val)
			{
				base::push(val);
			}

			value basic_eval_stack::pop()
			{
				return base::pop([](const value& v) { return v.is_none(); });
			}

			const value& basic_eval_stack::top() const
			{
				return base::top();
			}

			bool basic_eval_stack::is_empty() const
			{
				return base::is_empty();
			}

			void basic_eval_stack::clear()
			{
				base::clear();
			}

			void basic_eval_stack::mark_strings(string_table& strings) const
			{
				// Iterate everything (including what we have saved) and mark strings
				base::for_each_all([&strings](const value& elem) { elem.mark_strings(strings); });
			}

			void basic_eval_stack::save()
			{
				base::save();
			}

			void basic_eval_stack::restore()
			{
				base::restore();
			}

			void basic_eval_stack::forget()
			{
				// Clear out
				data x; x.set_none();
				value none = value(x);
				base::forget([&none](value& elem) { elem = none; });
			}

		}
	}
}