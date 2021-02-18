#pragma once

#include "executioner.h"
#include "operation_bases.h"

namespace ink::runtime::internal {
	template<value_type ty>
	class operation<Command::ADD, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() + vals[1].get<ty>() ));
		}
	}

	template<value_type ty>
	class operation<Command::SUBTRACT, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() - vals[1].get<ty>() ));
		}
	}

	template<value_type ty>
	class operation<Command::DIVIDE, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() / vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::MULTIPLY, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() * vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::MOD, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() % vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::IS_EQUAL, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() == vals[1].get<ty>()
			));
		}
	};

	template<value_type ty>
	class operation<Command::GREATER_THAN, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() > vals[1].get<ty>()
			));
		}
	};


	template<value_type ty>
	class operation<Command::LESS_THAN, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() < vals[1].get<ty>()
			));
		}
	};

	template<value_type ty>
	class operation<Command::GREATER_THAN_EQUALS, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() >= vals[1].get<ty>()
			));
		}
	};


	template<value_type ty>
	class operation<Command::LESS_THAN_EQUALS, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() <= vals[1].get<ty>()
			));
		}
	};

	template<value_type ty>
	class operation<Command::NOT_EQUAL, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() != vals[1].get<ty>()
			));
		}
	};

	template<value_type ty>
	class operation<Command::AND, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() && vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::OR, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() || vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::MIN, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(vals[0].get<ty>() < vals[1].get<ty>() ? vals[0] : vals[1]);
		}
	};

	template<value_type ty>
	class operation<Command::MAX, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(vals[0].get<ty>() > vals[1].get<ty>() ? vals[0] : vals[1]);
		}
	};

	template<value_type ty>
	class operation<Command::NOT, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>(!vals[0].get<ty>()));
		}
	}

	template<value_type ty>
	class operation<Command::NEGATE, ty> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>(-vals[0].get<ty>()));
		}
	}
}
