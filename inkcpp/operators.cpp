#include "pch.h"
#include "operators.h"

namespace binary
{
	namespace runtime
	{
		typedef value(*BinaryOperator)(Command command, const value& left, const value& right);

		static bool OperatorsInitialized = false;
		static BinaryOperator Operations[NUM_VALUE_TYPES];

		template<ValueType V>
		struct value_type_info { typedef void type; };

		template<>
		struct value_type_info<INTEGER> { typedef int type; };

		template<>
		struct value_type_info<FLOAT> { typedef float type; };

		template<>
		struct value_type_info<STRING> { typedef std::string type; };

		template<>
		struct value_type_info<DIVERT> { typedef int type; };

#include "operator_defs.h"

		template<ValueType V>
		value eval(Command command, const value& left, const value& right)
		{
			return evaluate<value_type_info<V>::type>(command, left.get<value_type_info<V>::type>(), right.get<value_type_info<V>::type>());
		}

		template<ValueType V>
		struct value_iterator
		{
			static void run()
			{
				Operations[V] = &eval<V>;
				value_iterator<(ValueType)(V + 1)>::run();
			}
		};

		template<>
		void value_iterator<NUM_VALUE_TYPES>::run() { } // empty

		void initialize_operators()
		{
			value_iterator<VALUE_TYPES_START>::run();
		}

		value run_operator(Command command, ValueType sharedType, const value& left, const value& right)
		{
			// Find the operator for this shared type
			BinaryOperator op = Operations[sharedType];
			if (op == nullptr)
				throw std::exception("Fuck 2");

			// Run it
			return op(command, left, right);
		}

		value evaluate(Command command, const value& left, const value& right)
		{
			if(!OperatorsInitialized)
				initialize_operators();

			ValueType tLeft = left.type(), tRight = right.type();

			// Same type? Just run the operator
			if (tLeft == tRight)
				return run_operator(command, tLeft, left, right);

			// Otherwise, need to cast up to the more important type
			ValueType castType = tLeft;
			if (tRight > castType)
				castType = tRight;

			// Run the operator by casting the types
			return run_operator(command, castType, left.cast(castType), right.cast(castType));
		}
	}
}