#include "functional.h"

#include "value.h"
#include "stack.h"
#include "string_table.h"

#ifdef INK_ENABLE_UNREAL
#include "InkVar.h"
#endif

namespace ink::runtime::internal
{
	template<typename T>
	T function_base::pop(basic_eval_stack* stack)
	{
		return stack->pop().get<T>();
	}

	template<typename T>
	void function_base::push(basic_eval_stack* stack, const T& value)
	{
		stack->push(value);
	}

	void function_base::push_string(basic_eval_stack* stack, const char* dynamic_string)
	{
		stack->push(value(dynamic_string, true));
	}

	char* function_base::allocate(string_table& strings, size_t len)
	{
		return strings.create(len);
	}

	// Generate template implementations for all significant types

#define SUPPORT_TYPE(TYPE) template TYPE function_base::pop<TYPE>(basic_eval_stack*); template void function_base::push<TYPE>(basic_eval_stack*, const TYPE&)
#define SUPPORT_TYPE_PARAMETER_ONLY(TYPE) template TYPE function_base::pop<TYPE>(basic_eval_stack*)

	SUPPORT_TYPE(int);
	SUPPORT_TYPE(float);
	SUPPORT_TYPE(uint32_t);

	// TODO - Support string return values

#ifdef INK_ENABLE_STL
	SUPPORT_TYPE_PARAMETER_ONLY(std::string);
#endif
#ifdef INK_ENABLE_UNREAL
	SUPPORT_TYPE_PARAMETER_ONLY(FString);

	template<>
	FInkVar function_base::pop<FInkVar>(basic_eval_stack* stack)
	{
		value v = stack->pop();
		switch (v.type())
		{
		case value_type::null:
		case value_type::divert:
			inkFail("Trying to pass null or divert as ink parameter to external function");
			break;
		case value_type::integer:
			return FInkVar(v.get<int>());
		case value_type::decimal:
			return FInkVar(v.get<float>());
		case value_type::string:
			return FInkVar(v.get<FString>());
		}

		return FInkVar();
	}

	template<>
	void function_base::push<FInkVar>(basic_eval_stack* stack, const FInkVar& value)
	{
		switch (value.type)
		{
		case EInkVarType::None:
			{
				internal::value v;
				stack->push(v);
			}
			break;
		case EInkVarType::Int:
			stack->push(value.intVar);
			break;
		case EInkVarType::Float:
			stack->push(value.floatVar);
			break;
		case EInkVarType::String:
			inkFail("NOT IMPLEMENTED"); // TODO: String support
			return;
		}
	}
#endif
}
