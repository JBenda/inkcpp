#include "stack.h"
#include "value.h"
#include "operations.h"

namespace ink::runtime::internal {
	
	float floor(float f) {
		if (f >= 0.f) {
			return static_cast<float>(static_cast<int>(f));
		}
		return static_cast<float>(static_cast<int>(f) - 1);
	}

	float ceil(float f) {
		if(f - floor(f) == 0) { return f; }
		return static_cast<float>(static_cast<int>(f) + 1);
	}

	void operation<Command::FLOOR, value_type::float32, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		inkAssert(vals[0].type() == value_type::float32, "Expected floating point number to floor.");
		stack.push(value{}.set<value_type::float32>(
					floor(vals->get<value_type::float32>())));
	}

	void operation<Command::CEILING, value_type::float32, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		inkAssert(vals[0].type() == value_type::float32, "Expected floating point number to ceil.");
		stack.push(value{}.set<value_type::float32>(
					ceil(vals->get<value_type::float32>())));
	}
}
