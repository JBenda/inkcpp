#pragma once

#include "value.h"
#include "system.h"
#include "output.h"
#include "stack.h"
#include "choice.h"
#include "config.h"

namespace ink
{
	namespace runtime
	{
		class story;
		class choice;

		class runner
		{
		public:
			// Creates a new runner at the start of a loaded ink story
			runner(const story*);

			// Checks that the runner can continue
			bool can_continue() const;
			inline operator bool() const { return can_continue(); }

			// == Choices ==

			// Does the runner have choices available
			bool has_choices() const { return _num_choices > 0; }

			// Number of choices
			size_t num_choices() const { return _num_choices; }

			// Gets a choice by index
			const choice& get_choice(size_t index);
			const choice& operator[](size_t index) { return get_choice(index); }

			// Chooses a choice by index
			void choose(int index);

			// Begin iterating choices
			const choice* begin() { return _choices; }

			// End iterating choices
			const choice* end() { return _choices + _num_choices; }

			// == Output ==

			// Gets a single line of output and allocates a C-style string 
			//  to store it in. This allocated string is not managed
			//  and must be deleted using delete[] by the user.
			char* getline_alloc();

#ifdef INK_ENABLE_STL
			// == STL Extensions ==

			// Gets a single line of output and stores it in a C++ std::string
			std::string getline();

			// Reads a line into a std::ostream
			void getline(std::ostream&);
#endif
		private:
			// Advances the interpreter by a line. This fills the output buffer
			void advance_line();

			// Steps the interpreter a single instruction and returns
			//  when it has hit a new line
			bool line_step();

			// Steps the interpreter a single instruction
			void step();

			// == Save/Restore
			void save();
			void restore();
			void forget();

			enum class change_type
			{
				no_change,
				extended_past_newline,
				newline_removed
			};

			change_type detect_change() const;

		private:
			template<typename T>
			inline T read();

			template<>
			inline const char* read();

			choice& add_choice();
			void clear_choices();

		private:
			const story* _story;

			// == State ==

			// Instruction pointer
			ip_t _ptr;
			ip_t _backup; // backup pointer

			// Output stream
			internal::stream<100> _output;

			// Runtime stack. Used to store temporary variables and callstack
			internal::stack<50> _stack;

			// Evaluation (NOTE: Will later need to be per-callstack entry)
			bool bEvaluationMode;
			internal::eval_stack<20> _eval;

			// Choice list
			static const size_t MAX_CHOICES = 10;
			choice _choices[MAX_CHOICES];
			size_t _num_choices;

			bool _finished;
			bool _saved;
		};

#ifdef INK_ENABLE_STL
		std::ostream& operator<<(std::ostream&, runner&);
#endif
	}
}