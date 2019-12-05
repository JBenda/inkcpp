#include "functional.h"

#include "value.h"
#include "stack.h"

namespace ink::runtime::internal
{
	template<typename T>
	static T function_base::pop(basic_eval_stack* stack)
	{
		return stack->pop().get<T>();
	}

	template<typename T>
	static void function_base::push(basic_eval_stack* stack, const T& value)
	{
		stack->push(value);
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
#endif
}