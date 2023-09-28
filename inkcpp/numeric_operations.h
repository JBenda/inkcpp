#pragma once
#include "value.h"

/// Define operation for numeric types.
/// use generalized types numeric and integral to keep redundancy minimal.
/// define a cast to support operations like int + float, bool + uint etc.

namespace ink::runtime::internal {

	/// list of numeric value types
	/// produces a SFINAE error if type is not part of list
	template<value_type ty>
	using is_numeric_t = typename enable_if<
		ty == value_type::boolean
		|| ty == value_type::int32
		|| ty == value_type::uint32
		|| ty == value_type::float32, void>::type;
	
	template<value_type ty>
	using is_signed_numeric_t = typename enable_if<
		ty == value_type::int32
		|| ty == value_type::float32, void>::type;

	/// list of internal value types
	/// produces a SFINAE error if type is not part of list
	template<value_type ty>
	using is_integral_t = typename enable_if<
		ty == value_type::boolean
		|| ty == value_type::int32
		|| ty == value_type::uint32, void>::type;

	namespace casting {
		/// define valid casts

		/// result of operation with int and float is float.
		template<>
		struct cast<value_type::int32, value_type::float32>
		{	static constexpr value_type value = value_type::float32; };

		/// result of operation with uint and bool is uint
		template<>
		struct cast<value_type::boolean, value_type::uint32>
		{ 	static constexpr value_type value = value_type::uint32; };
		
		// result of operation with bool and int is int
		template<>
		struct cast<value_type::boolean, value_type::int32>
		{ static constexpr value_type value = value_type::int32; };

		/// defined numeric cast
		/// generic numeric_cast only allow casting to its one type
		template<value_type to>
		inline typename value::ret<to>::type numeric_cast(const value& v) {
			if (to == v.type()) { return v.get<to>(); }
			else {
				inkFail("invalid numeric_cast! from %i to %i", v.type(), to);
				return 0;
			}
		}

		/// specialisation for uint32
		template<>
		inline typename value::ret<value_type::uint32>::type numeric_cast<value_type::uint32>(const value& v) {
			switch(v.type()) {
				case value_type::uint32:
					return v.get<value_type::uint32>();
				/// bool value can cast to uint32
				case value_type::boolean:
					return static_cast<uint32_t>(v.get<value_type::boolean>());
				default:
					inkFail("invalid cast to uint!");
					return 0;
			}
		}

		template<>
		inline typename value::ret<value_type::int32>::type numeric_cast<value_type::int32>(const value& v) {
			switch(v.type()) {
				case value_type::int32:
					return v.get<value_type::int32>();
				case value_type::boolean:
					return static_cast<int32_t>(v.get<value_type::boolean>());
				default:
					inkFail("invalid cast to int!");
					return 0;
			}
		}

		/// specialisation for float32
		template<>
		inline float numeric_cast<value_type::float32>(const value& v) {
			switch(v.type()) {
				case value_type::float32:
					return v.get<value_type::float32>();
				// int value can cast to float
				case value_type::int32:
					return static_cast<float>(v.get<value_type::int32>());
				default:
					inkFail("invalid numeric_cast!");
					return 0;
			}
		}

		/// specialisation for boolean
		template<>
		inline bool numeric_cast<value_type::boolean>(const value& v) {
			switch(v.type()) {
				case value_type::boolean:
					return v.get<value_type::boolean>();
				case value_type::int32:
					return v.get<value_type::int32>() != 0;
				case value_type::uint32:
					return v.get<value_type::uint32>() != 0;
				case value_type::float32:
					return v.get<value_type::float32>() != 0;
				default:
					inkFail("invalid numeric_cast to boolean from: %i", v.type());
					return false;
			}
		}
	}

	template<>
	class operation<Command::FLOOR, value_type::float32, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<>
	class operation<Command::CEILING, value_type::float32, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals);
	};

	template<value_type ty>
	class operation<Command::INT_CAST, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::int32>(
						static_cast<int32_t>(vals->get<ty>())
					));
		}
	};

	template<>
	class operation<Command::IS_EQUAL, value_type::divert, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
						vals[0].get<value_type::divert>()
						== vals[1].get<value_type::divert>()));
		}
	};
	template<>
	class operation<Command::NOT_EQUAL, value_type::divert, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
						vals[0].get<value_type::divert>()
						!= vals[1].get<value_type::divert>()));
		}
	};

	template<value_type ty>
	class operation<Command::FLOOR, ty, is_integral_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			// for integral types floor(i) == i
			stack.push(vals[0]);
		}
	};

	template<value_type ty>
	class operation<Command::CEILING, ty, is_integral_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			// for integral types ceil(i) == i
			stack.push(vals[0]);
		}
	};

	template<value_type ty>
	class operation<Command::ADD, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>(
						casting::numeric_cast<ty>(vals[0]) +
						casting::numeric_cast<ty>(vals[1]) ));
		}
	};

	template<value_type ty>
	class operation<Command::SUBTRACT, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>(
						casting::numeric_cast<ty>(vals[0]) -
						casting::numeric_cast<ty>(vals[1]) ));
		}
	};

	template<>
	class operation<Command::SUBTRACT, value_type::boolean, void> : public operation_base<void> {
		operation<Command::SUBTRACT, value_type::int32> op_int;
	public:
		template<typename T> operation(const T& t) : operation_base{t}, op_int{t} {}
		void operator()(basic_eval_stack& stack, value* vals) {
			op_int(stack, vals);
		}
	};

	template<value_type ty>
	class operation<Command::DIVIDE, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>(
						casting::numeric_cast<ty>(vals[0]) /
						casting::numeric_cast<ty>(vals[1]) ));
		}
	};
	
	template<>
	class operation<Command::DIVIDE, value_type::boolean, void> : public operation_base<void> {
		operation<Command::DIVIDE, value_type::int32> op_int;
	public:
		template<typename T> operation(const T& t) : operation_base{t}, op_int{t} {}
		void operator()(basic_eval_stack& stack, value* vals) {
			op_int(stack, vals);
		}
	};

	template<value_type ty>
	class operation<Command::MULTIPLY, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>(
						casting::numeric_cast<ty>(vals[0]) *
						casting::numeric_cast<ty>(vals[1]) ));
		}
	};

	template<value_type ty>
	class operation<Command::MOD, ty, is_integral_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>( 
				casting::numeric_cast<ty>(vals[0])
				% casting::numeric_cast<ty>(vals[1])));
		}
	};

	template<>
	class operation<Command::MOD, value_type::boolean, void> : public operation_base<void> {
		operation<Command::MOD, value_type::int32> op_int;
	public:
		template<typename T> operation(const T& t) : operation_base{t}, op_int{t} {}
		void operator()(basic_eval_stack& stack, value* vals) {
			op_int(stack, vals);
		}
	};

	template<value_type ty>
	class operation<Command::IS_EQUAL, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				casting::numeric_cast<ty>(vals[0]) ==
				casting::numeric_cast<ty>(vals[1])
			));
		}
	};

	template<value_type ty>
	class operation<Command::GREATER_THAN, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				casting::numeric_cast<ty>(vals[0]) >
				casting::numeric_cast<ty>(vals[1])
			));
		}
	};


	template<value_type ty>
	class operation<Command::LESS_THAN, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				casting::numeric_cast<ty>(vals[0]) <
				casting::numeric_cast<ty>(vals[1])
			));
		}
	};

	template<value_type ty>
	class operation<Command::GREATER_THAN_EQUALS, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				casting::numeric_cast<ty>(vals[0]) >=
				casting::numeric_cast<ty>(vals[1])
			));
		}
	};


	template<value_type ty>
	class operation<Command::LESS_THAN_EQUALS, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				casting::numeric_cast<ty>(vals[0]) <=
				casting::numeric_cast<ty>(vals[1])
			));
		}
	};

	template<value_type ty>
	class operation<Command::NOT_EQUAL, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(
				casting::numeric_cast<ty>(vals[0]) !=
				casting::numeric_cast<ty>(vals[1])
			));
		}
	};

	template<value_type ty>
	class operation<Command::AND, ty, is_integral_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>( 
				casting::numeric_cast<value_type::boolean>(vals[0])
				&& casting::numeric_cast<value_type::boolean>(vals[1])));
		}
	};

	template<value_type ty>
	class operation<Command::OR, ty, is_integral_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()( basic_eval_stack& stack, value* vals )
		{
			stack.push(value{}.set<value_type::boolean>( 
				casting::numeric_cast<value_type::boolean>(vals[0])
				|| casting::numeric_cast<value_type::boolean>(vals[1])));
		}
	};

	template<value_type ty>
	class operation<Command::MIN, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			typename value::ret<ty>::type n[2] = {
				casting::numeric_cast<ty>(vals[0]),
				casting::numeric_cast<ty>(vals[1])
			};
			stack.push(value{}.set<ty>(n[0] < n[1] ? n[0] : n[1]));
		}
	};

	template<value_type ty>
	class operation<Command::MAX, ty, is_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			typename value::ret<ty>::type n[2] = {
				casting::numeric_cast<ty>(vals[0]),
				casting::numeric_cast<ty>(vals[1])
			};
			stack.push(value{}.set<ty>(n[0] > n[1] ? n[0] : n[1]));
		}
	};

	template<value_type ty>
	class operation<Command::NOT, ty, is_integral_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(!vals[0].get<ty>()));
		}
	};

	template<value_type ty>
	class operation<Command::NEGATE, ty,  is_signed_numeric_t<ty>> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<ty>(-vals[0].get<ty>()));
		}
	};
	template<>
	class operation<Command::NEGATE, value_type::boolean, void> : public operation_base<void> {
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			stack.push(value{}.set<value_type::boolean>(!vals[0].get<value_type::boolean>()));
		}
	};

	template<>
	class operation<Command::RANDOM, value_type::int32, void> : public operation_base<prng>
	{
	public:
		using operation_base::operation_base;
		void operator()(basic_eval_stack& stack, value* vals) {
			int min = casting::numeric_cast<value_type::int32>(vals[0]);
			int max = casting::numeric_cast<value_type::int32>(vals[1]);
			stack.push(value{}.set<value_type::int32>(static_cast<int32_t>(_prng.rand(max - min + 1) + min)));
		}
	};
}
