#pragma once

namespace ink::runtime::internal {

	template<>
	class operation<Command::READ_COUNT_VAR, value_type::divert, void> : public operation_base<const story_impl, globals_impl>
	{
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::TURNS, value_type::divert, void> : public operation_base<const story_impl, globals_impl>
	{
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::CHOICE_COUNT, value_type::none, void> : public operation_base<const runner_interface>
	{
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};
}
