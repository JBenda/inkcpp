#include "stack.h"
#include "string_table.h"

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

	auto reverse_find_predicat_constructor(hash_t name, thread_t& skip, uint32_t& jumping) {
		return [name, &skip, &jumping](entry& e) {
			// Jumping
			if (jumping > 0) {
				jumping--;
				return false;
			}

			// If this is an end thread marker, skip over it
			if (skip == ~0 && e.data.type() == value_type::thread_end) {
				skip = e.data.get<value_type::thread_end>();
			}

			// If we're skipping
			if (skip != ~0) {
				// Stop if we get to the start of the thread block
				if (e.data.type() == value_type::thread_start && skip == e.data.get<value_type::thread_start>().jump) {
					skip = ~0;
				}

				// Don't return anything in the hidden thread block
				return false;
			}

			// Is it a thread start or a jump marker
			if (e.name == InvalidHash && (e.data.type() == value_type::thread_start || e.data.type() == value_type::jump_marker))
			{
				// If this thread start has a jump value
				uint32_t jump = e.data.get<value_type::jump_marker>().thread_id;

				// Then we need to do some jumping. Skip
				if (jump > 0) {
					jumping = jump;
					return false;
				}
			}

			return e.name == name || e.name == InvalidHash;
		};
	}

	const value* basic_stack::get(hash_t name) const {
		// Find whatever comes first: a matching entry or a stack frame entry
		thread_t skip = ~0;
		uint32_t jumping = 0;
		const entry* found = base::reverse_find(reverse_find_predicat_constructor(name, skip, jumping));

		// If nothing found, no value
		if (found == nullptr)
			return nullptr;

		// If we found something of that name, return the value
		if (found->name == name)
			return &found->data;

		// Otherwise, nothing in this stack frame
		return nullptr;
	}
	value* basic_stack::get(hash_t name) {
		// Find whatever comes first: a matching entry or a stack frame entry
		thread_t skip = ~0;
		uint32_t jumping = 0;
		entry* found = base::reverse_find(reverse_find_predicat_constructor(name, skip, jumping));

		// If nothing found, no value
		if (found == nullptr)
			return nullptr;

		// If we found something of that name, return the value
		if (found->name == name)
			return &found->data;

		// Otherwise, nothing in this stack frame
		return nullptr;
	}

	template<>
	void basic_stack::push_frame<frame_type::function>(offset_t return_to)
	{
		add(InvalidHash, value{}.set<value_type::function_frame>(return_to));
	}
	template<>
	void basic_stack::push_frame<frame_type::tunnel>(offset_t return_to)
	{
		add(InvalidHash, value{}.set<value_type::tunnel_frame>(return_to));
	}
	template<>
	void basic_stack::push_frame<frame_type::thread>(offset_t return_to)
	{
		add(InvalidHash, value{}.set<value_type::thread_frame>(return_to));
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
		value& start = threadIter.get()->data;
		value_type vt = start.type();
		auto jump = start.get<value_type::jump_marker>();

		// Move over it
		threadIter.next();

		// Move back over the current jump value
		for (uint32_t i = 0; i < jump.thread_id; i++)
			threadIter.next();

		// Now keep iterating back until we get to a frame marker
		// FIXME: meta types or subtypes?
		while (!threadIter.done() && (threadIter.get()->name != InvalidHash
					|| threadIter.get()->data.type() == value_type::thread_start
					|| threadIter.get()->data.type() == value_type::thread_end))
		{
			// If we've hit an end of thread marker
			auto e = threadIter.get();
			if (e->data.type() == value_type::thread_end)
			{
				// We basically want to skip until we get to the start of this thread (leave the block alone)
				thread_t tid = e->data.get<value_type::thread_end>();
				while (threadIter.get()->data.type() != value_type::thread_start
						|| threadIter.get()->data.get<value_type::thread_start>().jump != tid)
				{
					jump.thread_id++;
					threadIter.next();
				}

				// Now let us skip over the thread start
			}

			threadIter.next();
			jump.thread_id++;
		}

		// Move us over the frame marker
		jump.thread_id++;

		// Now that thread marker is set to the correct jump value.
		if (vt == value_type::jump_marker) {
			start.set<value_type::jump_marker>(jump);
		} else if (vt == value_type::thread_start) {
			start.set<value_type::thread_start>(jump);
		} else {
			throw ink_exception("unknown jump type");
		}
		return threadIter.get();
	}

	frame_type get_frame_type(value_type type)
	{
		switch (type)
		{
		case value_type::tunnel_frame:
			return frame_type::tunnel;
		case value_type::function_frame:
			return frame_type::function;
		case value_type::thread_frame:
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
			if (
			// FIXME: is_tghead_marker, is_jump_marker
					frame->data.type() == value_type::thread_start
				|| 	frame->data.type() == value_type::thread_end
				||  frame->data.type() == value_type::jump_marker
				)
			{
				// End of thread marker, we need to create a jump marker
				if (frame->data.type() == value_type::thread_end)
				{
					// Push a new jump marker after the thread end
					entry& jump = push({ InvalidHash, value{}.set<value_type::jump_marker>(0u,0u) });

					// Do a pop back
					returnedFrame = do_thread_jump_pop(base::begin());
					break;
				}

				// If this is a jump marker, we actually want to extend it to the next frame
				if (frame->data.type() == value_type::jump_marker)
				{
					// Use the thread jump pop method using this jump marker
					returnedFrame = do_thread_jump_pop(iter);
					break;
				}

				// Popping past thread start
				if (frame->data.type() == value_type::thread_start)
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
		inkAssert(returnedFrame->data.type() != value_type::thread_start
				&& returnedFrame->data.type() != value_type::thread_end,
			"Can not return from a thread! How did this happen?");

		// Store frame type
		if (type != nullptr)
		{
			*type = get_frame_type(returnedFrame->data.type());
		}

		// Return the offset stored in the frame record
		// FIXME: correct type?
		return returnedFrame->data.get<value_type::jump_marker>().jump;
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
				if (elem.data.type() == value_type::thread_start && elem.data.get<value_type::thread_start>().jump == thread)
					thread = ~0;

				return false;
			}

			// If it's a jump marker or a thread start
			if (elem.data.type() == value_type::jump_marker || elem.data.type() == value_type::thread_start) {
				jumping = elem.data.get<value_type::jump_marker>().thread_id;
				return false;
			}

			// If it's a thread end, we need to skip to the matching thread start
			if (elem.data.type() == value_type::thread_end) {
				thread = elem.data.get<value_type::thread_end>();
				return false;
			}

			return elem.name == InvalidHash;
		});

		if (frame != nullptr && returnType != nullptr)
			*returnType = get_frame_type(frame->data.type());

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
		base::for_each_all(
			[&strings](const entry& elem) {
				if (elem.data.type() == value_type::string) {
					strings.mark_used(elem.data.get<value_type::string>());
				}
		});
	}

	thread_t basic_stack::fork_thread()
	{
		// TODO create unique thread ID
		thread_t new_thread = _next_thread++;

		// Push a thread start marker here
		entry& thread_entry = add(InvalidHash, value{}.set<value_type::thread_start>(new_thread, 0u));

		// Set stack jump counter for thread to 0. This number is used if the thread ever
		//  tries to pop past its origin. It keeps track of how much of the preceeding stack it's popped back

		return new_thread;
	}

	void basic_stack::complete_thread(thread_t thread)
	{
		// Add a thread complete marker
		add(InvalidHash, value{}.set<value_type::thread_end>(thread));
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
				top->data.type() == value_type::thread_end &&
				top->data.get<value_type::thread_end>() == thread))
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
			if (nulling == ~0 && elem.data.type()  == value_type::thread_end && elem.name == InvalidHash) {
				nulling = elem.data.get<value_type::thread_end>();
			}

			// If we're deleting a useless thread block
			if (nulling != ~0) {
				// If this is the start of the block, stop deleting
				if (elem.name == InvalidHash && elem.data.type() == value_type::thread_start && elem.data.get<value_type::thread_start>().jump == nulling) {
					nulling = ~0;
				}

				// delete data
				elem.name = NulledHashId;
			}
			else
			{
				// Clear thread start markers. We don't need or want them anymore
				if (elem.name == InvalidHash &&
						(elem.data.type() == value_type::thread_start || elem.data.type() == value_type::jump_marker)) {
					// Clear it out
					elem.name = NulledHashId;

					// Check if this is a jump, if so we need to ignore even more data
					jumping = elem.data.get<value_type::jump_marker>().thread_id;
				}

				// Clear thread frame markers. We can't use them anymore
				if (elem.name == InvalidHash && elem.data.type() == value_type::thread_frame) {
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
		return base::pop([](const value& v) { return v.type() == value_type::none; });
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
		base::for_each_all([&strings](const value& elem) {
				if (elem.type() == value_type::string) {
					string_type str = elem.get<value_type::string>();
					if (str.allocated) {
						strings.mark_used(str.str);
					}
				}
			});
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
		value x; x.set<value_type::none>();
		value none = value(x);
		base::forget([&none](value& elem) { elem = none; });
	}
}
