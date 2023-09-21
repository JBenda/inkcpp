#pragma once

/// Define base constructs to specify by operation headers.

#include "command.h"
#include "stack.h"
#include "value.h"

namespace ink::runtime::internal {

	namespace casting {
		// default cast to none (invalid cast)
		template<value_type t1, value_type t2>
		struct cast {
			static constexpr value_type value = value_type::none;
		};

		// no cast for same type
		template<value_type t>
		struct cast<t,t> {
			static constexpr value_type value = t;
		};
	}

	/**
	 * @brief Determines the number of arguments needed for an command.
	 */
	constexpr size_t command_num_args(Command cmd) {
		if (cmd >= Command::TERNARY_OPERATORS_START && cmd <= Command::TERNARY_OPERATORS_END) {
			return 3;
		} else if (cmd >= Command::BINARY_OPERATORS_START && cmd <= Command::BINARY_OPERATORS_END) {
			return 2;
		} else if (cmd >= Command::UNARY_OPERATORS_START && cmd <= Command::UNARY_OPERATORS_END) {
			return 1;
		} else {
			return 0;
		}
	}

	/**
	 * @brief Operation definition.
	 * A class which contains a call operator to execute the operation needed
	 * for the command type combination.
	 * @tparam cmd Command which should be executed
	 * @tparam ty type on which the command should be executed
	 */
	template<Command cmd, value_type ty, typename enable = void>
	class operation {
	public:
		static constexpr bool enabled = false;
		template<typename T>
		operation(const T& t) {}
		/**
		 * @brief execute operation.
		 * @param stack were the result(s) get pushed
		 * @param vs array of values, first one = first argument etc
		 */
		void operator()(basic_eval_stack& stack, value* vs) {
			inkFail("operation not implemented!");
		}
	};
}

// include header here to ensure correct order

#include "operation_bases.h"
#include "numeric_operations.h"
#include "string_operations.h"
#include "list_operations.h"
#include "container_operations.h"
#include "casting.h"
