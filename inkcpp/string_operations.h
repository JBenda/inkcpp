#pragma once


namespace ink::runtime::internal {

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
