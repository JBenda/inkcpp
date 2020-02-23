#pragma once

#include "value.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			struct entry
			{
				hash_t name;
				value data;
			};

			class basic_stack
			{
			protected:
				basic_stack(entry* data, size_t size);

			public:

				// Sets existing value, or creates a new one at this callstack entry
				void set(hash_t name, const value& val);

				// Gets an existing value, or nullptr
				const value* get(hash_t name) const;

				// pushes a new frame onto the stack
				void push_frame(offset_t return_to);

				// Pops a frame (and all temporary variables) from the callstack.
				offset_t pop_frame();

				// Clears the entire stack
				void clear();

				// Garbage collection
				void mark_strings(string_table&) const;

				// == Save/Restore ==
				void save();
				void restore();
				void forget();

			private:
				void add(hash_t name, const value& val);
			private:
				// stack
				entry* _stack;
				size_t _size;

				// Current stack position
				size_t _pos;

				// Fuck me
				size_t _save;
				size_t _jump;
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

			class basic_eval_stack
			{
			protected:
				basic_eval_stack(value* data, size_t size);

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

			private:
				// stack
				value* _stack;
				size_t _size;

				// Current stack position
				size_t _pos;
			};

			template<size_t N>
			class eval_stack : public basic_eval_stack
			{
			public:
				eval_stack() : basic_eval_stack(&_stack[0], N) { }
			private:
				value _stack[N];
			};
		}
	}
}