#pragma once

/// defines operations on lists

namespace ink::runtime::internal {

	namespace casting {
		// define valid castings
		template<>
		struct cast<value_type::int32, value_type::list>
		{ static constexpr value_type value = value_type::list; };
		template<>
		struct cast<value_type::uint32, value_type::list>
		{ static constexpr value_type value = value_type::list; };
		template<>

		struct cast<value_type::int32, value_type::list_flag>
		{ static constexpr value_type value = value_type::list_flag; };
		template<>
		struct cast<value_type::uint32, value_type::list_flag>
		{ static constexpr value_type value = value_type::list_flag; };

		template<>
		struct cast<value_type::list, value_type::list_flag>
		{ static constexpr value_type value = value_type::list; };

		// opertions on mulitple list_flags results potential in a new list
		template<>
		struct cast<value_type::list_flag, value_type::list_flag>
		{ static constexpr value_type value = value_type::list; };

	}

	template<typename T>
	class redefine<value_type::list, list_table::list, T> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		value operator()(const list_table::list& lh, const list_table::list& rh)  {
			return value{}.set<value_type::list>(_list_table.redefine(lh,rh));
		}
	};

	template<>
	class operation<Command::ADD, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::SUBTRACT, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::ADD, value_type::list_flag, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::SUBTRACT, value_type::list_flag, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};


	template<>
	class operation<Command::INTERSECTION, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LIST_COUNT, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};
	template<>
	class operation<Command::LIST_COUNT, value_type::list_flag, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LIST_MIN, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};
	template<>
	class operation<Command::LIST_MIN, value_type::list_flag, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LIST_MAX, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};
	template<>
	class operation<Command::LIST_MAX, value_type::list_flag, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::lrnd, value_type::list, void> : public operation_base<list_table, prng> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};
	template<>
	class operation<Command::lrnd, value_type::list_flag, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LIST_ALL, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};
	template<>
	class operation<Command::LIST_ALL, value_type::list_flag, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LIST_INVERT, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};
	template<>
	class operation<Command::LIST_INVERT, value_type::list_flag, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LESS_THAN, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::GREATER_THAN, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};


	template<>
	class operation<Command::GREATER_THAN_EQUALS, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LESS_THAN_EQUALS, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	
	template<>
	class operation<Command::IS_EQUAL, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::NOT_EQUAL, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::HAS, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::HASNT, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LIST_VALUE, value_type::list_flag, void>: public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			inkAssert(vals[0].type() == value_type::list_flag, "LIST_VALUE only works on list_flag values");
			stack.push(value{}.set<value_type::int32>(static_cast<int32_t>(
							vals[0].get<value_type::list_flag>().flag) + 1));
		}
	};

	template<>
	class operation<Command::LIST_INT, value_type::string, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::LIST_RANGE, value_type::list, void> : public operation_base<list_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};
}

