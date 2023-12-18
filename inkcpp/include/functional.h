#pragma once

#include "config.h"
#include "list.h"
#include "traits.h"
#include "system.h"
#include "types.h"

#ifdef INK_ENABLE_UNREAL
#	include "../InkVar.h"
#endif

namespace ink::runtime::internal
{
class basic_eval_stack;
class string_table;
class list_table;

class callback_base
{
public:
	virtual void call(ink::runtime::value, ink::optional<ink::runtime::value>) = 0;
};

template<typename F>
class callback final : public callback_base
{
	using traits = function_traits<F>;
	static_assert(traits::arity < 3);

	template<ink::runtime::value::Type Ty>
	void call_functor(ink::runtime::value new_val, ink::optional<ink::runtime::value> old_val)
	{
		if constexpr (traits::arity == 2) {
			if (old_val.has_value()) {
				functor(
				    new_val.get<Ty>(),
				    typename traits::template argument<1>::type{old_val.value().get<Ty>()}
				);
			} else {
				functor(new_val.get<Ty>(), ink::nullopt);
			}
		} else {
			functor(new_val.get<Ty>());
		}
	}

public:
	callback(const callback&)            = delete;
	callback(callback&&)                 = delete;
	callback& operator=(const callback&) = delete;
	callback& operator=(callback&&)      = delete;

	callback(F functor)
	    : functor(functor)
	{
	}

	virtual void call(ink::runtime::value new_val, ink::optional<ink::runtime::value> old_val)
	{
		using value     = ink::runtime::value;
		auto check_type = [&new_val, &old_val](value::Type type) {
			inkAssert(
			    new_val.type == type, "Missmatch type for variable observer: expected %i, got %i",
			    static_cast<int>(type), static_cast<int>(new_val.type)
			);
			if constexpr (traits::arity == 2) {
				// inkAssert(!old_val.has_value() || old_val.value().type == type,
				// 	"Missmatch type for variable observers old value: expected optional<%i> got
				// optional<%i>", static_cast<int>(type), static_cast<int>(old_val.value().type));
			}
		};
		if constexpr (traits::arity > 0) {
			using arg_t = typename remove_cvref<typename traits::template argument<0>::type>::type;
			if constexpr (is_same<arg_t, value>::value) {
				if constexpr (traits::arity == 2) {
					functor(new_val, old_val);
				} else {
					functor(new_val);
				}
			} else if constexpr (is_same<arg_t, bool>::value) {
				check_type(value::Type::Bool);
				call_functor<value::Type::Bool>(new_val, old_val);
			} else if constexpr (is_same<arg_t, uint32_t>::value) {
				check_type(value::Type::Uint32);
				call_functor<value::Type::Uint32>(new_val, old_val);
			} else if constexpr (is_same<arg_t, int32_t>::value) {
				check_type(value::Type::Int32);
				call_functor<value::Type::Int32>(new_val, old_val);
			} else if constexpr (is_same<arg_t, const char*>::value) {
				check_type(value::Type::String);
				call_functor<value::Type::String>(new_val, old_val);
			} else if constexpr (is_same<arg_t, float>::value) {
				check_type(value::Type::Float);
				call_functor<value::Type::Float>(new_val, old_val);
			} else if constexpr (is_same<arg_t, list_interface*>::value) {
				check_type(value::Type::List);
				call_functor<value::Type::List>(new_val, old_val);
			} else {
				static_assert(
				    always_false<arg_t>::value, "Unsupported value for variable observer callback!"
				);
			}
		} else {
			functor();
		}
	}

private:
	F functor;
};

// base function container with virtual callback methods
class function_base
{
public:
	virtual ~function_base() {}

	// calls the underlying function object taking parameters from a stack
#ifdef INK_ENABLE_UNREAL
	virtual void
	    call(basic_eval_stack* stack, size_t length, string_table& strings, list_table& lists)
	    = 0;
#else
	virtual void
	    call(basic_eval_stack* stack, size_t length, string_table& strings, list_table& lists)
	    = 0;
#endif

protected:
	// used to hide basic_eval_stack and value definitions
	template<typename T>
	static T pop(basic_eval_stack* stack, list_table& lists);

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
	function(F functor)
	    : functor(functor)
	{
	}

	// calls the underlying function using arguments on the stack
	virtual void call(
	    basic_eval_stack* stack, size_t length, string_table& strings, list_table& lists
	) override
	{
		call(stack, length, strings, lists, GenSeq<traits::arity>());
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
	arg_type<index> pop_arg(basic_eval_stack* stack, list_table& lists)
	{
		// todo - type assert?
		return pop<arg_type<index>>(stack, lists);
	}

	static constexpr bool is_array_call()
	{
		if constexpr (traits::arity == 2) {
			return is_same<
			    const ink::runtime::value*, typename traits::template argument<1>::type>::value;
		}
		return false;
	}

	template<size_t... Is>
	void
	    call(basic_eval_stack* stack, size_t length, string_table& strings, list_table& lists, seq<Is...>)
	{
		inkAssert(
		    is_array_call() || sizeof...(Is) == length,
		    "Attempting to call functor with too few/many arguments"
		);
		static_assert(sizeof...(Is) == traits::arity);
		if_t<is_array_call(), value[config::maxArrayCallArity], char> vals;
		if constexpr (is_array_call()) {
			inkAssert(
			    length <= config::maxArrayCallArity,
			    "AIttampt to call array call with more arguments then supportet, please change in "
			    "config"
			);
			for (size_t i = 0; i < length; ++i) {
				vals[i] = pop<ink::runtime::value>(stack, lists);
			}
		}
		// void functions
		if constexpr (is_same<void, typename traits::return_type>::value) {
			// Just evaluevaluatelate
			if constexpr (is_array_call()) {
				functor(length, vals);
			} else {
				functor(pop_arg<Is>(stack, lists)...);
			}

			// Ink expects us to push something
			push_void(stack);
		} else {
			typename traits::return_type res;
			if constexpr (is_array_call()) {
				res = functor(length, vals);
			} else {
				res = functor(pop_arg<Is>(stack, lists)...);
			}
			if constexpr (is_string<typename traits::return_type>::value) {
				// SPECIAL: The result of the functor is a string type
				//  in order to store it in the inkcpp interpreter we
				//  need to store it in our allocated string table
				// Get string length
				size_t len = string_handler<typename traits::return_type>::length(res);

				// Get source and allocate buffer
				char* buffer = allocate(strings, len + 1);
				string_handler<typename traits::return_type>::src_copy(res, buffer);

				// push string result
				push_string(stack, buffer);
			} else if constexpr (is_same<value, remove_cvref<typename traits::return_type>>::value) {
				if (res.type() == ink::runtime::value::Type::String) {
					auto   src    = res.template get<ink::runtime::value::Type::String>();
					size_t len    = string_handler<decltype(src)>::length(src);
					char*  buffer = allocate(strings, len + 1);
					string_handler<decltype(src)>::src_copy(src, buffer);
					push_string(stack, buffer);
				} else {
					push(stack, res);
				}
			} else {
				// Evaluate and push the result onto the stack
				push(stack, res);
			}
		}
	}
};

#ifdef INK_ENABLE_UNREAL
template<typename D>
class function_array_delegate : public function_base
{
public:
	function_array_delegate(const D& del)
	    : invocableDelegate(del)
	{
	}

	// calls the underlying delegate using arguments on the stack
	virtual void call(
	    basic_eval_stack* stack, size_t length, string_table& strings, list_table& lists
	) override
	{
		constexpr bool RET_VOID
		    = is_same<typename function_traits<decltype(&D::Execute)>::return_type, void>::value;
		// Create variable array
		TArray<FInkVar> variables;
		for (size_t i = 0; i < length; i++) {
			variables.Add(pop<FInkVar>(stack, lists));
		}
		if constexpr (RET_VOID) {
			invocableDelegate.Execute(variables);
			push(stack, 0);
		} else {

			auto                ret    = invocableDelegate.Execute(variables);
			ink::runtime::value result = ret.to_value();
			if (result.type == ink::runtime::value::Type::String) {
				const char* src    = result.v_string;
				size_t      len    = string_handler<const char*>::length(src);
				char*       buffer = allocate(strings, len + 1);
				char*       ptr    = buffer;
				while (*src != '\0')
					*(ptr++) = *(src++);
				*ptr            = 0;
				result.v_string = buffer;
			}
			push(stack, result);
		}
	}

private:
	D invocableDelegate;
};
#endif
} // namespace ink::runtime::internal
