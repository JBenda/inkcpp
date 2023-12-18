#include "functional.h"

#include "value.h"
#include "stack.h"
#include "string_table.h"

#ifdef INK_ENABLE_UNREAL
#	include "InkVar.h"
#endif

namespace ink::runtime::internal
{
template<>
ink::runtime::value
    function_base::pop<ink::runtime::value>(basic_eval_stack* stack, list_table& lists)
{
	value val = stack->pop();
	return val.to_interface_value(lists);
}

template<>
int32_t function_base::pop<int32_t>(basic_eval_stack* stack, list_table& lists)
{
	value val = stack->pop();
	inkAssert(val.type() == value_type::int32, "Type missmatch!");
	return val.get<value_type::int32>();
}

template<>
const char* function_base::pop<const char*>(basic_eval_stack* stack, list_table& lists)
{
	value val = stack->pop();
	inkAssert(val.type() == value_type::string, "Type missmatch!");
	return val.get<value_type::string>().str;
}

template<>
void function_base::push<int32_t>(basic_eval_stack* stack, const int32_t& v)
{
	stack->push(value{}.set<value_type::int32>(v));
}

void function_base::push_void(basic_eval_stack* stack) { stack->push(values::null); }

void function_base::push_string(basic_eval_stack* stack, const char* dynamic_string)
{
	stack->push(value{}.set<value_type::string>(dynamic_string, true));
}

char* function_base::allocate(string_table& strings, size_t len) { return strings.create(len); }

// Generate template implementations for all significant types

#ifdef INK_ENABLE_STL
template<>
std::string function_base::pop<std::string>(basic_eval_stack* stack, list_table& lists)
{
	return std::string(pop<const char*>(stack, lists));
}
#endif
#ifdef INK_ENABLE_UNREAL
template<>
FInkVar function_base::pop<FInkVar>(basic_eval_stack* stack, list_table& lists)
{
	return FInkVar(stack->pop().to_interface_value(lists));
}
#endif
template<>
void function_base::push<ink::runtime::value>(
    basic_eval_stack* stack, const ink::runtime::value& value
)
{
	internal::value val{};
	if (val.set(value)) {
		stack->push(val);
	} else {
		inkFail("unable to set variable?");
	}
}
} // namespace ink::runtime::internal
