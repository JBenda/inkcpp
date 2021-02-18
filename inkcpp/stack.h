#pragma once

#include "value.h"
#include "collections/restorable.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			class string_table;
			struct entry
			{
				hash_t name;
				value data;
			};

			enum class frame_type : uint32_t
			{
				function,
				tunnel,
				thread
			};

			class basic_stack : protected restorable<entry>
			{
			protected:
				basic_stack(entry* data, size_t size);

				// base class
				using base = restorable<entry>;

			public:
				// Sets existing value, or creates a new one at this callstack entry
				void set(hash_t name, const value& val);

				// Gets an existing value, or nullptr
				const value* get(hash_t name) const;
				value* get(hash_t name);

				// pushes a new frame onto the stack
				template<frame_type>
				void push_frame(offset_t return_to);

				// Pops a frame (and all temporary variables) from the callstack.
				offset_t pop_frame(frame_type* type);

				// Returns true if there are any frames on the stack
				bool has_frame(frame_type* type = nullptr) const;

				// Clears the entire stack
				void clear();

				// Garbage collection
				void mark_strings(string_table&) const;

				// == Threading ==

				// Forks a new thread from the current callstack and returns that thread's unique id
				thread_t fork_thread();

				// Mark a thread as "done". It's callstack is still preserved until collapse_to_thread is called.
				void complete_thread(thread_t thread);

				// Collapses the callstack to the state of a single thread
				void collapse_to_thread(thread_t thread);

				// == Save/Restore ==
				void save();
				void restore();
				void forget();

			private:
				entry& add(hash_t name, const value& val);
				const entry* pop();

				entry* do_thread_jump_pop(const iterator& jump);

				// thread ids
				thread_t _next_thread = 0;
				thread_t _backup_next_thread = 0;

				static const hash_t NulledHashId = ~0;
			};

			// stack for call history and temporary variables
			template<size_t N>
			class stack : public basic_stack
			{
			public:
				stack() : basic_stack(&_stack[0], N) { }
			private:
				// stack
				entry _stack[N];
			};

			class basic_eval_stack : protected restorable<value>
			{
			protected:
				basic_eval_stack(value* data, size_t size);

				using base = restorable<value>;

			public:
				// Push value onto the stack
				void push(const value&);

				// Pop a value off the stack
				value pop();

				// Gets the top value without popping
				const value& top() const;

				// Check if the stack is empty
				bool is_empty() const;

				// Clear stack
				void clear();

				// Garbage collection
				void mark_strings(string_table&) const;

				// == Save/Restore ==
				void save();
				void restore();
				void forget();
			};

			class eval_stack : public basic_eval_stack
			{
				static constexpr size_t N = 20;
			public:
				eval_stack() : basic_eval_stack(&_stack[0], N) { }
			private:
				value _stack[N];
			};
		}
	}
}
