#pragma once

#include "executioner.h"
#include "operation_bases.h"

namespace ink::runtime::internal {

	template<value_type ty>
	class operation<Command::ADD, ty> : operation_base<string_table> {
	public:
		using operation_base::operation_base;
		void operator()(eval_stack& stack, value* vals) {
		}
	}
}
