#pragma once

/// defines operations allowed on strings.

namespace ink::runtime::internal {

	namespace casting {
		// define valid castings
		// when operate on float and string, the result is a string
		template<>
		struct cast<value_type::float32, value_type::string>
		{ static constexpr value_type value = value_type::string; };
		template<>
		struct cast<value_type::int32, value_type::string>
		{ static constexpr value_type value = value_type::string; };
		template<>
		struct cast<value_type::uint32, value_type::string>
		{ static constexpr value_type value = value_type::string; };
		template<>
		struct cast<value_type::string, value_type::newline>
		{ static constexpr value_type value = value_type::string; };
	}

	// operation declaration add
	template<>
	class operation<Command::ADD, value_type::string, void> : public operation_base<string_table> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	// operation declaration equality
	template<>
	class operation<Command::IS_EQUAL, value_type::string, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::NOT_EQUAL, value_type::string, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::HAS, value_type::string, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::HASNT, value_type::string, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

}
