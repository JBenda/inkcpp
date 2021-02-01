#pragma once

#include "value.h"
#include "system.h"
#include "output.h"
#include "stack.h"
#include "choice.h"
#include "config.h"
#include "simple_restorable_stack.h"
#include "types.h"
#include "functions.h"
#include "string_table.h"
#include "array.h"

#include "runner.h"
#include "choice.h"

namespace ink::runtime::internal
{
	class story_impl;
	class globals_impl;

	class runner_impl : public runner_interface
	{
	public:
		// Creates a new runner at the start of a loaded ink story
		runner_impl(const story_impl*, globals);
		virtual ~runner_impl();

		// used by the globals object to do garbage collection
		void mark_strings(string_table&) const;

#pragma region runner Implementation

		// Checks that the runner can continue
		virtual bool can_continue() const override;

		// Begin iterating choices
		virtual const choice* begin() const override { return _choices; }

		// End iterating choices
		virtual const choice* end() const override { return _choices + _num_choices; }

		// Chooses a choice by index
		virtual void choose(size_t index) override;

		// runs silently
		void getline_silent();

#ifdef INK_ENABLE_CSTD
		// c-style getline
		virtual char* getline_alloc() override;
#endif

		// move to path
		virtual bool move_to(hash_t path) override;

#ifdef INK_ENABLE_STL
		// Gets a single line of output and stores it in a C++ std::string
		virtual std::string getline() override;

		// Reads a line into a std::ostream
		virtual void getline(std::ostream&) override;

		// get all into string
		virtual std::string getall() override;

		// get all into stream
		virtual void getall(std::ostream&) override;
#endif
#ifdef INK_ENABLE_UNREAL
		// Reads a line into an Unreal FString
		virtual FString getline() override;
#endif
#pragma endregion
	protected:
		// bind external
		virtual void internal_bind(hash_t name, internal::function_base* function) override;
	private:
		// Advances the interpreter by a line. This fills the output buffer
		void advance_line();

		// Steps the interpreter a single instruction and returns
		//  when it has hit a new line
		bool line_step();

		// Steps the interpreter a single instruction
		void step();

		// Resets the runtime
		void reset();

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

		choice& add_choice();
		void clear_choices();

		// Special code for jumping from the current IP to another
		void jump(ip_t, bool record_visits = true);

		void run_binary_operator(unsigned char cmd);
		void run_unary_operator(unsigned char cmd);

		frame_type execute_return();

		void on_done(bool setDone);
		void set_done_ptr(ip_t ptr);

		inline thread_t current_thread() const { return _threads.empty() ? ~0 : _threads.top(); }

	private:
		const story_impl* const _story;
		story_ptr<globals_impl> _globals;

		// == State ==

		// Instruction pointer
		ip_t _ptr;
		ip_t _backup; // backup pointer
		ip_t _done; // when we last hit a done

		// Output stream
		internal::stream<200> _output;

		// Runtime stack. Used to store temporary variables and callstack
		internal::stack<50> _stack;

		// Evaluation stack
		bool bEvaluationMode = false;
		internal::eval_stack<20> _eval;
		bool bSavedEvaluationMode = false;

		// Keeps track of what threads we're inside
		internal::restorable_stack<thread_t, 20> _threads;
		internal::fixed_restorable_array<ip_t, 20> _threadDone;

		// Choice list
		static const size_t MAX_CHOICES = 10;
		choice _choices[MAX_CHOICES];
		size_t _num_choices = 0;

		// TODO: Move to story? Both?
		functions _functions;

		// Container set
		internal::restorable_stack<container_t, 20> _container;
		bool _is_falling = false;

		bool _saved = false;
	};

	template<>
	inline const char* runner_impl::read();

#ifdef INK_ENABLE_STL
	std::ostream& operator<<(std::ostream&, runner_impl&);
#endif
}
