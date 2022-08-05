#pragma once

/// Defines the executioner class which initialize the different operations
/// and managed the access to them.
///
/// The executer creates a array of pointer to the arguments passed, and pass
/// them to each operator, so that each operator can grep the needed arguments.
/// Therefore it is required that each argument has a unique type, so that the
/// order won't matter.
///
/// When call an operation the executioner iterates through all commands and
/// after find an command match.
/// Then pop arguments from the stack as defined in `command_num_args`.
/// After this iterate through the implementations of that command for different
/// type until it found the correct type, than execute the operation.
/// The search is O(n), but the list is only populated with commands which have
/// at least one implementation, also per command only types listed for which
/// the command is implemented.
///
/// Improvements: The executioner -> typed_executer could be O(1) when using a
/// look up table.

#include "system.h"
#include "value.h"
#include "stack.h"
#include "operations.h"



namespace ink::runtime::internal {

	/**
	 * @brief iterates through value_types until it found a matching operator.
	 * Matching means a operator which implements the command for the type.
	 * @tparam cmd Command to search operation for.
	 * @tparam t value type to start search
	 * @tparam Offset t + Offset is real start, used because trouble with mscv
	 * @return value_type::OP_END, if no "next operation" found
	 * @return type which is greater t + Offset and implement the command
	 */
	template<Command cmd, value_type t, size_t Offset>
	constexpr value_type next_operatable_type() {
		constexpr value_type ty = t + Offset;
		if constexpr (operation<cmd,ty>::enabled) {
			return ty;
		} else if constexpr (ty >= value_type::OP_END){
			return value_type::OP_END;
		} else {
			return next_operatable_type<cmd,ty,1>();
		}
	}

	/**
	 * @brief Iterates through all existing operations for this Command.
	 */
	template<Command cmd, value_type ty = next_operatable_type<cmd,value_type::BEGIN,0>()>
	class typed_executer {
	public:
		static constexpr bool enabled = true;
		template<typename T>
		typed_executer(const T& t) : _typed_exe{t}, _op{t} {}

		void operator()(value_type t, basic_eval_stack& s, value* v) {
			if (t == ty) { _op(s, v); }
			else { _typed_exe(t, s, v); }
		}
	private:
		// skip command for not implemented types
		typed_executer<cmd, next_operatable_type<cmd,static_cast<value_type>(ty),1>()> _typed_exe;
		operation<cmd, ty> _op;
	};

	// end of recursion (has no operation attached to it)
	template<Command cmd>
	class typed_executer<cmd, value_type::OP_END> {
	public:
		static constexpr bool enabled = false;
		template<typename T>
		typed_executer(const T& t) {}

		void operator()(value_type, basic_eval_stack&, value*) {
			inkFail("Operation for value not supported!");
		}
	};

	/**
	 * @brief Find next command which is at least for one type implemented.
	 * @tparam c command to start search
	 * @tparam Offset offset to start search, used because of trouble with mscv
	 * @return Command::OP_END if no next operation is found
	 * @return next command witch at least of implementation.
	 */
	template<Command c, size_t Offset>
	constexpr Command next_operatable_command() {
		constexpr Command cmd = c + Offset;
		if constexpr (typed_executer<cmd>::enabled) {
			return cmd;
		} else if constexpr (cmd >= Command::OP_END){
			return Command::OP_END;
		} else {
			return next_operatable_command<cmd,1>();
		}
	}

	/**
	 * @brief  Iterate through all commands to find correct command.
	 * Also instantiates all typed_executer and with them the operations.
	 */
	template<Command cmd = next_operatable_command<Command::OP_BEGIN,0>()>
	class executer_imp {
	public:
		template<typename T>
		executer_imp(const T& t) : _exe{t}, _typed_exe{t}{}

		void operator()(Command c, basic_eval_stack& s) {
			if (c == cmd) {
				static constexpr size_t N = command_num_args(cmd);
				if constexpr (N == 0) {
					value_type ty  = casting::common_base<0>(nullptr);
					_typed_exe(ty, s, nullptr);
				} else {
					value args[N];
					for (int i = command_num_args(cmd)-1; i >= 0 ; --i) {
						args[i] = s.pop();
					}
					value_type ty = casting::common_base<N>(args);
					_typed_exe(ty, s, args);
				}
			} else { _exe(c, s); }
		}
	private:
		executer_imp<next_operatable_command<cmd,1>()> _exe;
		typed_executer<cmd> _typed_exe;
	};

	/// end of recursion
	template<>
	class executer_imp<Command::OP_END> {
	public:
		template<typename T>
		executer_imp(const T& t) {}
		void operator()(Command, basic_eval_stack&) {
			inkFail("requested command was not found!");
		}
	};

	/**
	 * @brief Class which instantiates all operations and give access to them.
	 */
	class executer {
	public:
		/**
		 * @brief pass all arguments to operations who need them.
		 * @attention each type need to be unique for the look up later!
		 * @tparam Args argument types
		 * @param args arguments
		 */
		template<typename ... Args>
		executer(Args& ... args) : _executer{tuple<Args*...>(&args...)} {}

		/**
		 * @brief execute command on stack.
		 * @param cmd command to execute
		 * @param stack stack to operate on
		 */
		void operator()(Command cmd, basic_eval_stack& stack) {
			_executer(cmd, stack);
		}
	private:
	executer_imp<Command::OP_BEGIN> _executer;
	};
}

