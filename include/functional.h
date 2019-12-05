#pragma once

#include "traits.h"
#include "../inkcpp/value.h" // TODO!!
#include <utility> // TODO!!
#include "../inkcpp/stack.h"

namespace ink::runtime::internal
{
	struct function_base
	{
		virtual value call(basic_eval_stack* stack, size_t length) = 0;
		virtual ~function_base() { }
	};

	template<typename F>
	struct function : function_base
	{
		F functor;
		function(F functor) : functor(functor) { }

		using traits = function_traits<F>;

		template<int index>
		using XX = typename function_traits<F>::argument<index>::type;

		template<int index>
		typename XX<index> x(basic_eval_stack* stack)
		{
			return stack->pop().get<XX<index>>();
		}

		template<size_t... Is>
		value call(basic_eval_stack* stack, size_t length, std::index_sequence<Is...>)
		{
			inkAssert(sizeof...(Is) == length, "Attempting to call functor with too few/many arguments");
			static_assert(sizeof...(Is) == traits::arity);

			if constexpr (std::is_same<void, traits::return_type>::value)
			{
				functor(x<Is>(stack)...);
				return 0;
			}
			else
			{
				return functor(x<Is>(stack)...);
			}
		}

		virtual value call(basic_eval_stack* stack, size_t length) override
		{
			return call(stack, length, std::make_index_sequence<traits::arity>());
		}
	};
}