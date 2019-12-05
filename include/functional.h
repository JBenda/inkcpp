#pragma once

#include "traits.h"

namespace ink::runtime::internal
{
	class basic_eval_stack;

	// base function container with virtual callback methods
	class function_base
	{
	public:
		virtual ~function_base() { }

		// calls the underlying function object taking parameters from a stack
		virtual void call(basic_eval_stack* stack, size_t length) = 0;

	protected:
		// used to hide basic_eval_stack and value definitions
		template<typename T>
		static T pop(basic_eval_stack* stack);

		// used to hide basic_eval_stack and value definitions
		template<typename T>
		static void push(basic_eval_stack* stack, const T& value);
	};

	// Stores a Callable function object and forwards calls to it
	template<typename F>
	class function : public function_base
	{
	public:
		function(F functor) : functor(functor) { }

		// calls the underlying function using arguments on the stack
		virtual void call(basic_eval_stack* stack, size_t length) override
		{
			call(stack, length, GenSeq<traits::arity>());
		}

	private:
		// Callable functor object
		F functor;

		// function traits
		using traits = function_traits<F>;

		// argument types
		template<int index>
		using arg_type = typename function_traits<F>::argument<index>::type;

		// pops an argument from the stack using the function-type
		template<int index>
		typename arg_type<index> pop_arg(basic_eval_stack* stack)
		{
			// todo - type assert?

			return pop<arg_type<index>>(stack);
		}

		template<size_t... Is>
		void call(basic_eval_stack* stack, size_t length, seq<Is...>)
		{
			// Make sure the argument counts match
			inkAssert(sizeof...(Is) == length, "Attempting to call functor with too few/many arguments");
			static_assert(sizeof...(Is) == traits::arity);

			// void functions
			if constexpr (is_same<void, traits::return_type>::value)
			{
				// Just evalulate
				functor(pop_arg<Is>(stack)...);
				
				// Ink expects us to push something
				// TODO -- Should be a special "void" value
				push(stack, 0);
			}
			else
			{
				// Evaluate and push the result onto the stack
				push(stack, functor(pop_arg<Is>(stack)...));
			}
		}
	};
}