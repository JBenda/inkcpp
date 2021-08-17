#include "functional.h"

#include "value.h"
#include "stack.h"
#include "string_table.h"

#ifdef INK_ENABLE_UNREAL
#include "InkVar.h"
#endif

namespace ink::runtime::internal
{
	template<>
	int32_t function_base::pop<int32_t>(basic_eval_stack* stack)
	{
		value val = stack->pop();
		inkAssert(val.type() == value_type::int32, "Type missmatch!");
		return val.get<value_type::int32>();
	}

	template<>
	void function_base::push<int32_t>(basic_eval_stack* stack, const int32_t& v)
	{
		stack->push(value{}.set<value_type::int32>(v));
	}

	void function_base::push_string(basic_eval_stack* stack, const char* dynamic_string)
	{
		stack->push(value{}.set<value_type::string>(dynamic_string, true));
	}

	char* function_base::allocate(string_table& strings, size_t len)
	{
		return strings.create(len);
	}

	// Generate template implementations for all significant types

#ifdef INK_ENABLE_STL
	template<>
	std::string function_base::pop<std::string>(basic_eval_stack* stack) {
		return std::string(pop<const char*>(stack));
	}
#endif
#ifdef INK_ENABLE_UNREAL
	template<>
	FString function_base::pop<FString>(basic_eval_stack* stack) {
		return FString(pop<const char*>(stack));
	}

	template<>
	FInkVar function_base::pop<FInkVar>(basic_eval_stack* stack)
	{
		value v = stack->pop();
		switch (v.type())
		{
		case value_type::int32:
			return FInkVar(v.get<value_type::int32>());
		case value_type::float32:
			return FInkVar(v.get<value_type::float32>());
		case value_type::string:
			return FInkVar(FString(v.get<value_type::string>().str));
		}

		inkFail("You can only pass integers, floats, or strings into ink functions.");
		return FInkVar();
	}

	template<>
	void function_base::push<FInkVar>(basic_eval_stack* stack, const FInkVar& value)
	{
		internal::value v;

		switch (value.type)
		{
		case EInkVarType::None:
			// Set to none
			break;
		case EInkVarType::Int:
			v.set<value_type::int32>(value.intVar);
			break;
		case EInkVarType::Float:
			v.set<value_type::float32>(value.floatVar);
			break;
		case EInkVarType::String:
			inkFail("NOT IMPLEMENTED"); // TODO: String support
			return;
		}

		stack->push(v);
	}
#endif
}
