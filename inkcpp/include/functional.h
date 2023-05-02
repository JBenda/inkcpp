#pragma once

#include "traits.h"
#include "system.h"

#ifdef  INK_ENABLE_UNREAL
#include "../InkVar.h"
#endif
namespace ink::runtime::internal
{
	class basic_eval_stack;
	class string_table;

	// base function container with virtual callback methods
	class function_base
	{
	public:
		virtual ~function_base() { }

		// calls the underlying function object taking parameters from a stack
#ifdef INK_ENABLE_UNREAL
		virtual void call(basic_eval_stack* stack, size_t length, string_table& strings) = 0;
#else
		virtual void call(basic_eval_stack* stack, size_t length, string_table& strings) = 0;
#endif

	protected:
		// used to hide basic_eval_stack and value definitions
		template<typename T>
		static T pop(basic_eval_stack* stack);

		// used to hide basic_eval_stack and value definitions
		template<typename T>
		static void push(basic_eval_stack* stack, const T& value);

		static void push_void(basic_eval_stack* stack);

		// string special push
		static void push_string(basic_eval_stack* stack, const char* dynamic_string);

		// used to hide string_table definitions
		static char* allocate(string_table& strings, ink::size_t len);
	};

	// Stores a Callable function object and forwards calls to it
	template<typename F>
	class function : public function_base
	{
	public:
		function(F functor) : functor(functor) { }

		// calls the underlying function using arguments on the stack
		virtual void call(basic_eval_stack* stack, size_t length, string_table& strings) override
		{
			call(stack, length, strings, GenSeq<traits::arity>());
		}

	private:
		// Callable functor object
		F functor;

		// function traits
		using traits = function_traits<F>;

		// argument types
		template<int index>
		using arg_type = typename function_traits<F>::template argument<index>::type;

		// pops an argument from the stack using the function-type
		template<int index>
		arg_type<index> pop_arg(basic_eval_stack* stack)
		{
			// todo - type assert?

			return pop<arg_type<index>>(stack);
		}

		template<size_t... Is>
		void call(basic_eval_stack* stack, size_t length, string_table& strings, seq<Is...>)
		{
			// Make sure the argument counts match
			inkAssert(sizeof...(Is) == length, "Attempting to call functor with too few/many arguments");
			static_assert(sizeof...(Is) == traits::arity);

			// void functions
			if constexpr (is_same<void, typename traits::return_type>::value)
			{
				// Just evaluevaluatelate
				functor(pop_arg<Is>(stack)...);
				
				// Ink expects us to push something
				// TODO -- Should be a special "void" value
				push_void(stack);
			}
			else if constexpr (is_string<typename traits::return_type>::value)
			{
				// SPECIAL: The result of the functor is a string type
				//  in order to store it in the inkcpp interpreter we 
				//  need to store it in our allocated string table
				auto string_result = functor(pop_arg<Is>(stack)...);

				// Get string length
				size_t len = string_handler<typename traits::return_type>::length(string_result);

				// Get source and allocate buffer
				const char* src = string_handler<typename traits::return_type>::src(string_result);
				char* buffer = allocate(strings, len + 1);

				// Copy
				char* ptr = buffer;
				while (*src != '\0')
					*(ptr++) = *(src++);
				*ptr = 0;

				// push string result
				push_string(stack, buffer);
			}
			else
			{
				// Evaluate and push the result onto the stack
				push(stack, functor(pop_arg<Is>(stack)...));
			}
		}
	};

#ifdef INK_ENABLE_UNREAL
	template<typename D>
	class function_array_delegate : public function_base
	{
	public:
		function_array_delegate(const D& del) : invocableDelegate(del) { }

		// calls the underlying delegate using arguments on the stack
		virtual void call(basic_eval_stack* stack, size_t length, string_table& strings) override
		{
			constexpr bool RET_VOID = 
				is_same<typename function_traits<decltype(&D::Execute)>::return_type,
						void>::value;
			// Create variable array
			TArray<FInkVar> variables;
			for (size_t i = 0; i < length; i++)
			{
				variables.Add(pop<FInkVar>(stack));
			}
            if constexpr (RET_VOID)
			{
				invocableDelegate.Execute(variables);
				push(stack, 0);
			} else {
				
				auto ret = invocableDelegate.Execute(variables);
				ink::runtime::value result = ret.to_value();
				if(result.type == ink::runtime::value::Type::String) {
					const char* src = result.v_string;
					size_t len = string_handler<const char*>::length(src);
					char* buffer = allocate(strings, len + 1);
					char* ptr = buffer;
					while(*src != '\0')
						*(ptr++) = *(src++);
					*ptr = 0;
					result.v_string = buffer;
				}
				push(stack, result);
			}
		}
	private:
		D invocableDelegate;
	};
#endif
}
