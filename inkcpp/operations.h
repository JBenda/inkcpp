#pragma once

#include "../shared/private/command.h"

namespace ink::runtime::internal {

	namespace casting {
		template<value_type t1, value_type t2>
		constexpr value_type cast = value_type::none;
		template<value_type t>
		constexpr value_type cast<t,t> = t;
	}

	constexpr size_t command_num_args(Command cmd) {
		if (cmd >= Command::BINARY_OPERATORS_START && cmd <= Command::BINARY_OPERATORS_END) {
			return 2;
		} else if (cmd >= Command::UNARY_OPERATORS_START && cmd <= Command::UNARY_OPERATORS_END) {
			return 1;
		} else {
			return 0;
		}
	}
	template<Command cmd>
	static constexpr size_t CommandNumArguments = command_num_args(cmd);

	template<Command cmd, value_type ty, typename enable = void>
	class operation {
	public:
		static constexpr bool enabled = false;
		template<typename T>
		operation(const T& t) {}
		void operator()(eval_stack&, value*) {
			throw ink_exception("operation not implemented!");
		}
	};
}

#include "operation_bases.h"
#include "numeric_operations.h"
/* #include "marker_operations.h" */
#include "string_operations.h"
#include "casting.h"
