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
		uint32_t jumping = 0;
		const entry* found = base::reverse_find([name, &skip, &jumping](entry& e) {
			// Jumping
			if (jumping > 0) {
				jumping--;
				return false;
			}

			// If this is an end thread marker, skip over it
			if (skip == ~0 && e.data.get_data_type() == data_type::thread_end) {
				skip = e.data.as_divert();
			}

			// If we're skipping
			if (skip != ~0) {
				// Stop if we get to the start of the thread block
				if (e.data.get_data_type() == data_type::thread_start && skip == e.data.as_divert()) {
					skip = ~0;
				}

				// Don't return anything in the hidden thread block
				return false;
			}

			// Is it a thread start or a jump marker
			if (e.name == InvalidHash && (e.data.get_data_type() == data_type::thread_start || e.data.get_data_type() == data_type::jump_marker))
			{
				// If this thread start has a jump value
				uint32_t jump = e.data.thread_jump();

				// Then we need to do some jumping. Skip
				if (jump > 0) {
					jumping = jump;
					return false;
				}
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
		data_type frameDataType;
		switch (type)
		{
		case frame_type::tunnel:
			frameDataType = data_type::tunnel_frame;
			break;
		case frame_type::function:
			frameDataType = data_type::function_frame;
			break;
		case frame_type::thread:
			frameDataType = data_type::thread_frame;
			break;
		}

		// Add to top of stack
		add(InvalidHash, value(return_to, frameDataType));
	}

	const entry* basic_stack::pop()
	{
		return &base::pop([](const entry& elem) { return elem.name == ~0; });
	}

	entry* basic_stack::do_thread_jump_pop(const basic_stack::iterator& jumpStart)
	{
		// Start an iterator right after the jumping marker (might be a thread_start or a jump_marker)
		iterator threadIter = jumpStart;

		// Get a reference to its jump count
		uint32_t& jump = threadIter.get()->data.thread_jump();

		// Move over it
		threadIter.next();

		// Move back over the current jump value
		for (uint32_t i = 0; i < jump; i++)
			threadIter.next();

		// Now keep iterating back until we get to a frame marker
		while (!threadIter.done() && (threadIter.get()->name != InvalidHash || threadIter.get()->data.is_thread_marker()))
		{
			// If we've hit an end of thread marker
			auto e = threadIter.get();
			if (e->data.is_thread_end())
			{
				// We basically want to skip until we get to the start of this thread (leave the block alone)
				thread_t tid = e->data.as_thread_id();
				while (!threadIter.get()->data.is_thread_start() || threadIter.get()->data.as_thread_id() != tid)
				{
					jump++;
					threadIter.next();
				}

				// Now let us skip over the thread start
			}

			threadIter.next();
			jump++;
		}

		// Move us over the frame marker
		jump++;

		// Now that thread marker is set to the correct jump value.
		return threadIter.get();
	}

	frame_type get_frame_type(data_type type)
	{
		switch (type)
		{
		case data_type::tunnel_frame:
			return frame_type::tunnel;
		case data_type::function_frame:
			return frame_type::function;
		case data_type::thread_frame:
			return frame_type::thread;
		default:
			inkAssert(false, "Unknown frame type detected");
			return (frame_type)-1;
		}
	}

	offset_t basic_stack::pop_frame(frame_type* type)
	{
		inkAssert(!base::is_empty(), "Can not pop frame from empty callstack.");

		const entry* returnedFrame = nullptr;

		// Start iterating backwards
		iterator iter = base::begin();
		while (!iter.done())
		{
			// Keep popping if it's not a frame marker or thread marker of some kind
			entry* frame = iter.get();
			if (frame->name != InvalidHash)
			{
				pop();
				iter = base::begin();
				continue;
			}

			// We now have a frame marker. Check if it's a thread
			// Thread handling
			if (frame->data.is_thread_marker() || frame->data.is_jump_marker())
			{
				// End of thread marker, we need to create a jump marker
				if (frame->data.get_data_type() == data_type::thread_end)
				{
					// Push a new jump marker after the thread end
					entry& jump = push({ InvalidHash, value(0, data_type::jump_marker) });
					jump.data.thread_jump() = 0;

					// Do a pop back
					returnedFrame = do_thread_jump_pop(base::begin());
					break;
				}

				// If this is a jump marker, we actually want to extend it to the next frame
				if (frame->data.is_jump_marker())
				{
					// Use the thread jump pop method using this jump marker
					returnedFrame = do_thread_jump_pop(iter);
					break;
				}

				// Popping past thread start
				if (frame->data.get_data_type() == data_type::thread_start)
				{
					returnedFrame = do_thread_jump_pop(iter);
					break;
				}
			}

			// Otherwise, pop the frame marker off and return it
			returnedFrame = pop();
			break;
		}

		// If we didn't find a frame entry, we never had a frame to return from
		inkAssert(returnedFrame, "Attempting to pop_frame when no frames exist! Stack reset.");

		// Make sure we're not somehow trying to "return" from a thread
		inkAssert(returnedFrame->data.get_data_type() != data_type::thread_start && returnedFrame->data.get_data_type() != data_type::thread_end,
			"Can not return from a thread! How did this happen?");

		// Store frame type
		if (type != nullptr)
		{
			*type = get_frame_type(returnedFrame->data.get_data_type());
		}

		// Return the offset stored in the frame record
		return returnedFrame->data.as_divert();
	}

	bool basic_stack::has_frame(frame_type* returnType) const
	{
		// Empty case
		if (base::is_empty())
			return false;

		uint32_t jumping = 0;
		uint32_t thread = ~0;
		// Search in reverse for a stack frame
		const entry* frame = base::reverse_find([&jumping, &thread](const entry& elem) {
			// If we're jumping over data, just keep returning false until we're done
			if (jumping > 0) {
				jumping--;
				return false;
			}

			// We only care about elements with InvalidHash
			if (elem.name != InvalidHash)
				return false;

			// If we're skipping over a thread, wait until we hit its start before checking
			if (thread != ~0) {
				if (elem.data.is_thread_start() && elem.data.as_thread_id() == thread)
					thread = ~0;

				return false;
			}

			// If it's a jump marker or a thread start
			if (elem.data.is_jump_marker() || elem.data.is_thread_start()) {
				jumping = elem.data.thread_jump();
				return false;
			}

			// If it's a thread end, we need to skip to the matching thread start
			if (elem.data.is_thread_end()) {
				thread = elem.data.as_thread_id();
				return false;
			}

			return elem.name == InvalidHash;
		});

		if (frame != nullptr && returnType != nullptr)
			*returnType = get_frame_type(frame->data.get_data_type());

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
		entry& thread_entry = add(InvalidHash, value(new_thread, data_type::thread_start));

		// Set stack jump counter for thread to 0. This number is used if the thread ever
		//  tries to pop past its origin. It keeps track of how much of the preceeding stack it's popped back
		thread_entry.data.thread_jump() = 0;

		return new_thread;
	}

	void basic_stack::complete_thread(thread_t thread)
	{
		// Add a thread complete marker
		add(InvalidHash, value(thread, data_type::thread_end));
	}

	void basic_stack::collapse_to_thread(thread_t thread)
	{
		// Reset thread counter
		_next_thread = 0;

		// If we're restoring a specific thread (and not the main thread)
		if (thread != ~0)
		{
			// Keep popping until we find the requested thread's end marker
			const entry* top = pop();
			while (!(
				top->data.get_data_type() == data_type::thread_end &&
				top->data.as_divert() == thread))
			{
				inkAssert(!is_empty(), "Ran out of stack while searching for end of thread marker. Did you call complete_thread?");
				top = pop();
			}
		}

		// Now, start iterating backwards
		thread_t nulling = ~0;
		uint32_t jumping = 0;
		base::reverse_for_each([&nulling, &jumping](entry& elem) {
			if (jumping > 0) {
				// delete data
				elem.name = NulledHashId;

				// Move on
				jumping--;
				return;
			}

			// Thread end. We just need to delete this whole block
			if (nulling == ~0 && elem.data.is_thread_end() && elem.name == InvalidHash) {
				nulling = elem.data.as_divert();
			}

			// If we're deleting a useless thread block
			if (nulling != ~0) {
				// If this is the start of the block, stop deleting
				if (elem.name == InvalidHash && elem.data.get_data_type() == data_type::thread_start && elem.data.as_divert() == nulling) {
					nulling = ~0;
				}

				// delete data
				elem.name = NulledHashId;
			}
			else
			{
				// Clear thread start markers. We don't need or want them anymore
				if (elem.name == InvalidHash && (elem.data.is_thread_start() || elem.data.is_jump_marker())) {
					// Clear it out
					elem.name = NulledHashId;

					// Check if this is a jump, if so we need to ignore even more data
					jumping = elem.data.thread_jump();
				}

				// Clear thread frame markers. We can't use them anymore
				if (elem.name == InvalidHash && elem.data.get_data_type() == data_type::thread_frame) {
					elem.name = NulledHashId;
				}
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

	entry& basic_stack::add(hash_t name, const value& val)
	{
		return base::push({ name, val });
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