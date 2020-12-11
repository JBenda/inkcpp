#include "stack.h"

namespace ink::runtime::internal
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
		thread_t skip = ~0;
		const entry* found = base::reverse_find([name, &skip](entry& e) {
			// If this is an end thread marker, skip over it
			if (e.data.data_type() == data_type::thread_end) {
				skip = e.data.as_divert();
			}

			// If we're skipping
			if (skip != ~0) {
				// Stop if we get to the start of the thread block
				if (e.data.data_type() == data_type::thread_start) {
					skip = ~0;
				}

				// Don't return anything in the hidden thread block
				return false;
			}

			return e.name == name || e.name == InvalidHash;
		});

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

	const entry* basic_stack::pop()
	{
		return &base::pop([](const entry& elem) { return elem.name == ~0; });
	}

	offset_t basic_stack::pop_frame(frame_type* type)
	{
		inkAssert(!base::is_empty(), "Can not pop frame from empty callstack.");

		// Keep popping until we find a frame entry
		const entry* popped = pop();
		while (popped->name != InvalidHash && !base::is_empty())
		{
			popped = pop();
		}

		// If we didn't find a frame entry, we never had a frame to return from
		inkAssert(popped->name == InvalidHash, "Attempting to pop_frame when no frames exist! Stack reset.");

		// Make sure we're not somehow trying to "return" from a thread
		inkAssert(popped->data.data_type() != data_type::thread_start && popped->data.data_type() != data_type::thread_end, 
			"Can not return from a thread! How did this happen?");

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

	thread_t basic_stack::fork_thread()
	{
		// TODO create unique thread ID
		thread_t new_thread = _next_thread++;

		// Push a thread start marker here
		// TODO: Back counter
		add(InvalidHash, value(new_thread, data_type::thread_start));

		return new_thread;
	}

	void basic_stack::complete_thread(thread_t thread)
	{
		// Add a thread complete marker
		add(InvalidHash, value(thread, data_type::thread_end));
	}

	void basic_stack::collapse_to_thread(thread_t thread)
	{
		// If we're restoring a specific thread (and not the main thread)
		if (thread != ~0)
		{
			// Keep popping until we find the requested thread's end marker
			const entry* top = pop();
			while (!(
				top->data.data_type() == data_type::thread_end &&
				top->data.as_divert() == thread))
			{
				top = pop();
			}
		}

		// Now, start iterating backwards
		thread_t nulling = ~0;
		base::reverse_for_each([&nulling](entry& elem) {

			// Thread end. We just need to delete this whole block
			if (nulling == ~0 && elem.data.data_type() == data_type::thread_end) {
				nulling = elem.data.as_divert();
			}

			// If we're deleting a useless thread block
			if (nulling != ~0) {
				// If this is the start of the block, stop deleting
				if (elem.data.data_type() == data_type::thread_start && elem.data.as_divert() == nulling) {
					nulling = ~0;
				}

				// delete data
				elem.name = NulledHashId;
			}
			else
			{
				// Clear thread start markers. We don't need or want them anymore
				if (elem.data.data_type() == data_type::thread_start) {
					elem.name = NulledHashId;
				}

				// TODO: Threads with negative offsets

				// TODO: Jump markers!
			}

		}, [](entry& elem) { return elem.name == NulledHashId; });

		// No more threads. Clear next thread counter
		_next_thread = 0;
	}

	void basic_stack::save()
	{
		base::save();

		// Save thread counter
		_backup_next_thread = _next_thread;
	}

	void basic_stack::restore()
	{
		base::restore();

		// Restore thread counter
		_next_thread = _backup_next_thread;
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