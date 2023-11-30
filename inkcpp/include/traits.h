#pragma once

#include "config.h"
#include "system.h"

#ifdef INK_ENABLE_STL
#include <string>
#endif

namespace ink::runtime::internal
{
	template<typename ... Ts>
	constexpr size_t sizeof_largest_type() 
	{
		size_t ret = 0;
		return ( (ret = sizeof(Ts) > ret ? sizeof(Ts) : ret), ... );
	}

	template<unsigned int N, typename Arg, typename... Args>
	struct get_ith_type : get_ith_type<N - 1, Args...> {};

	template<typename Arg, typename... Args>
	struct get_ith_type<0, Arg, Args...>
	{
		using type = Arg;
	};

	// constant and is_same from http://www.cppreference.com
	template<typename T, T v>
	struct constant {
		static constexpr T value = v;
		typedef T value_type;
		typedef constant type; // using injected-class-name
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; } //since c++14
	};

	struct false_type : constant<bool, false> {};
	struct true_type : constant<bool, true>{};

	template<typename B>
	true_type test_ptr_conv(const volatile B*);
	template<typename>
	false_type test_ptr_conv(const volatile void*);
	template<typename B, typename D>
	auto test_is_base_of(int) -> decltype(test_ptr_conv<B>(static_cast<D*>(nullptr)));
	// template<typename, typename> /// FIXME: needed?
	// auto test_is_base_of(...) -> true_type;

	template<class Base, class Derived>
	struct is_base_of : constant<bool, decltype(test_is_base_of<Base, Derived>(0))::value> {};

	template<class T, class U>
	struct is_same : false_type {};

	template<class T>
	struct is_same<T, T> : true_type {};

	template<typename T>
	struct is_pointer : false_type {};

	template<typename T>
	struct is_pointer<T*> : true_type {};

	template<class T> struct remove_cv { typedef T type; };
	template<class T> struct remove_cv<const T> { typedef T type; };
	template<class T> struct remove_cv<volatile T> { typedef T type; };
	template<class T> struct remove_cv<const volatile T> { typedef T type; };
	template<class T>
	struct remove_cvref
	{ typedef std::remove_cv_t<std::remove_reference_t<T>> type; };


	// == string testing (from me) ==

	template<typename T>
	struct is_string : false_type { };

	template<typename T>
	struct is_string<T&> : is_string<T> { };

	template<typename T>
	struct is_string<const T> : is_string<T> { };

	template<typename T>
	struct is_string<const T*> : is_string<T*> { };

	template<typename T>
	struct string_handler { };

	template<typename T>
	struct string_handler<const T> : string_handler<T> { };

	template<typename T>
	struct string_handler<const T*> : string_handler<T*> { };

	template<typename T>
	struct string_handler<T&> : string_handler<T> { };

#define MARK_AS_STRING(TYPE, LEN, SRC) template<> struct is_string<TYPE> : constant<bool, true> { }; \
	template<> struct string_handler<TYPE> { \
		static size_t length(const TYPE& x) { return LEN; } \
		static void src_copy(const TYPE& x, char* output) { \
			[&output](const char* src){\
			while(*src != '\0')  *(output++) = *(src++); \
			*output = 0; \
			}(SRC);\
		} \
	}

	inline size_t c_str_len(const char* c) {
		const char* i = c;
		while (*i != 0)
			i++;
		return i - c;
	}

	MARK_AS_STRING(char*, c_str_len(x), x);
#ifdef INK_ENABLE_STL
	MARK_AS_STRING(std::string, x.size(), x.c_str());
#endif
#ifdef INK_ENABLE_UNREAL
	MARK_AS_STRING(FString, x.Len(), TCHAR_TO_UTF8(*x));
#endif

#undef MARK_AS_STRING

	// function_traits from https://functionalcpp.wordpress.com/2013/08/05/function-traits/

	template<class F>
	struct function_traits;

	// function pointer
	template<class R, class... Args>
	struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)>
	{};

	template<class R, class... Args>
	struct function_traits<R(Args...)>
	{
		using return_type = R;

		static constexpr unsigned int arity = sizeof...(Args);

		template <unsigned int N>
		struct argument
		{
			static_assert(N < arity, "error: invalid parameter index.");
			using type = typename get_ith_type<N, Args...>::type;
		};
	};

	// member function pointer
	template<class C, class R, class... Args>
	struct function_traits<R(C::*)(Args...)> : public function_traits<R(C&, Args...)>
	{};

	// const member function pointer
	template<class C, class R, class... Args>
	struct function_traits<R(C::*)(Args...) const> : public function_traits<R(C&, Args...)>
	{};

	// member object pointer
	template<class C, class R>
	struct function_traits<R(C::*)> : public function_traits<R(C&)>
	{};

	// functor
	template<class F>
	struct function_traits
	{
	private:
		using call_type = function_traits<decltype(&F::operator())>;
	public:
		using return_type = typename call_type::return_type;

		static constexpr unsigned int arity = call_type::arity - 1;

		template <unsigned int N>
		struct argument
		{
			static_assert(N < arity, "error: invalid parameter index.");
			using type = typename call_type::template argument<N + 1>::type;
		};
	};

	// from https://stackoverflow.com/questions/17424477/implementation-c14-make-integer-sequence
	// using aliases for cleaner syntax
	template<class T> using Invoke = typename T::type;

	template<unsigned...> struct seq { using type = seq; };

	template<class S1, class S2> struct concat;

	template<unsigned... I1, unsigned... I2>
	struct concat<seq<I1...>, seq<I2...>>
		: seq<I1..., (sizeof...(I1) + I2)...> {};

	template<class S1, class S2>
	using Concat = Invoke<concat<S1, S2>>;

	template<unsigned N> struct gen_seq;
	template<unsigned N> using GenSeq = Invoke<gen_seq<N>>;

	template<unsigned N>
	struct gen_seq : Concat<GenSeq<N / 2>, GenSeq<N - N / 2>> {};

	template<> struct gen_seq<0> : seq<> {};
	template<> struct gen_seq<1> : seq<0> {};
}
