#pragma once

namespace ink::runtime::internal {
	template<value_type ty>
	class operation<Command::ADD, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() + vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::SUBTRACT, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() - vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::DIVIDE, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() / vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::MULTIPLY, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() * vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::MOD, ty, is_integral_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( vals[0].get<ty>() % vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::IS_EQUAL, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() == vals[1].get<ty>()
			));
		}
	};

	template<value_type ty>
	class operation<Command::GREATER_THAN, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() > vals[1].get<ty>()
			));
		}
	};


	template<value_type ty>
	class operation<Command::LESS_THAN, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() < vals[1].get<ty>()
			));
		}
	};

	template<value_type ty>
	class operation<Command::GREATER_THAN_EQUALS, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() >= vals[1].get<ty>()
			));
		}
	};


	template<value_type ty>
	class operation<Command::LESS_THAN_EQUALS, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() <= vals[1].get<ty>()
			));
		}
	};

	template<value_type ty>
	class operation<Command::NOT_EQUAL, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				vals[0].get<ty>() != vals[1].get<ty>()
			));
		}
	};

	template<value_type ty>
	class operation<Command::AND, ty, is_integral_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>( vals[0].get<ty>() && vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::OR, ty, is_integral_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>( vals[0].get<ty>() || vals[1].get<ty>() ));
		}
	};

	template<value_type ty>
	class operation<Command::MIN, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(vals[0].get<ty>() < vals[1].get<ty>() ? vals[0] : vals[1]);
		}
	};

	template<value_type ty>
	class operation<Command::MAX, ty, is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(vals[0].get<ty>() > vals[1].get<ty>() ? vals[0] : vals[1]);
		}
	};

	template<value_type ty>
	class operation<Command::NOT, ty, is_integral_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(!vals[0].get<ty>()));
		}
	};

	template<value_type ty>
	class operation<Command::NEGATE, ty,  is_numeric_t<ty>> : operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>(-vals[0].get<ty>()));
		}
	};
}
