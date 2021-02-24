#pragma once

#include "value.h"
#include "stack.h"
#include "operations.h"

#include <tuple>



namespace ink::runtime::internal {

	template<Command cmd, value_type ty>
	constexpr value_type next_operatable_type() {
		if constexpr (operation<cmd,ty>::enabled) {
			return ty;
		} else if constexpr (ty >= value_type::OP_END){
			return value_type::OP_END;
		} else {
			return next_operatable_type<cmd,ty+1>();
		}
	}
	template<Command cmd, value_type ty = next_operatable_type<cmd,value_type::BEGIN>()>
	class typed_executer {
	public:
		template<typename T>
		typed_executer(const T& t) : _typed_exe{t}, _op{t} {}

		void operator()(value_type t, eval_stack& s, value* v) {
			if (t == ty) { _op(s, v); }
			else { _typed_exe(t, s, v); }
		}
	private:
		typed_executer<cmd, next_operatable_type<cmd,ty+1>()> _typed_exe;
		operation<cmd, ty> _op;
	};
	template<Command cmd>
	class typed_executer<cmd, value_type::OP_END> {
	public:
		template<typename T>
		typed_executer(const T& t) {}

		void operator()(value_type, eval_stack&, value*) {
			throw ink_exception("Operation for value not supported!");
		}
	};

	template<Command cmd = Command::OP_BEGIN>
	class executer_imp {
	public:
		template<typename T>
		executer_imp(const T& t) : _exe{t}, _typed_exe{t}{}

		void operator()(Command c, eval_stack& s) {
			if (c == cmd) {
				static constexpr size_t N = CommandNumArguments<cmd>;
				value args[N];
				for (int i = CommandNumArguments<cmd>-1; i >= 0 ; --i) {
					args[i] = s.pop();
				}
				value_type ty = casting::common_base<N>(args);
				_typed_exe(ty, s, args);
			} else { _exe(c, s); }
		}
	private:
		executer_imp<cmd + 1> _exe;
		typed_executer<cmd> _typed_exe;
	};
	template<>
	class executer_imp<Command::OP_END> {
	public:
		template<typename T>
		executer_imp(const T& t) {}
		void operator()(Command, eval_stack&) {
			throw ink_exception("requested command was not found!");
		}
	};

	class executer {
	public:
		template<typename ... Args>
		executer(Args& ... args) : _executer{std::tuple<Args*...>(&args...)} {}
		void operator()(Command cmd, eval_stack& stack) {
			_executer(cmd, stack);
		}
	private:
	executer_imp<Command::OP_BEGIN> _executer;
	};
}

