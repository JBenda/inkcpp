#pragma once


namespace ink::runtime::internal {

	namespace casting {
		template<>
		constexpr value_type cast<value_type::float32, value_type::string> = value_type::string;
		template<>
		constexpr value_type cast<value_type::int32, value_type::string> = value_type::string;
		template<>
		constexpr value_type cast<value_type::uint32, value_type::string> = value_type::string;
	}

	template<>
	class operation<Command::ADD, value_type::string, void> : public operation_base<string_table> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::IS_EQUAL, value_type::string, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals);
	};
}
