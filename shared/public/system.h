#pragma once

#include "config.h"
#include "traits.h"

#ifdef INK_ENABLE_UNREAL
#include "Misc/AssertionMacros.h"
#include "Misc/CString.h"
#include "HAL/UnrealMemory.h"
#include "Hash/CityHash.h"
#endif
#ifdef INK_ENABLE_STL
#include <exception>
#include <stdexcept>
#include <optional>
#include <tuple>
#endif

namespace ink
{
	typedef unsigned int uint32_t;

	// Name hash (used for temporary variables)
	typedef uint32_t hash_t;

	// Invalid hash
	const hash_t InvalidHash = 0;

	// Simple hash for serialization of strings
#ifdef INK_ENABLE_UNREAL
	inline hash_t hash_string(const char* string)
	{
		return CityHash32(string, FCStringAnsi::Strlen(string));
	}
#else
	hash_t hash_string(const char* string);
#endif

	// Byte type
	typedef unsigned char byte_t;

	// Used to identify an offset in a data table (like a string in the string table)
	typedef uint32_t offset_t;

	// Instruction pointer used for addressing within the story instructions
	typedef unsigned char const* ip_t;
	
	// Used for the size of arrays
	typedef unsigned int size_t;

	// Used as the unique identifier for an ink container
	typedef uint32_t container_t;

	// Used to uniquely identify threads
	typedef uint32_t thread_t;

	// Checks if a string is only whitespace
	static bool is_whitespace(const char* string, bool includeNewline = true)
	{
		// Iterate string
		while (true)
		{
			switch (*(string++))
			{
			case 0:
				return true;
			case '\n':
				if (!includeNewline)
					return false;
			case '\t':
			case ' ':
				continue;
			default:
				return false;
			}
		}
	}

	static bool is_whitespace(char character, bool includeNewline = true)
	{
		switch (character)
		{
		case '\n':
			if (!includeNewline)
				return false;
		case '\t':
		case ' ':
			return true;
		default:
			return false;
		}
	}

	// Zero memory
#ifndef INK_ENABLE_UNREAL
	void zero_memory(void* buffer, size_t length);
#endif

	// assert	
#ifndef INK_ENABLE_UNREAL
	void ink_assert(bool condition, const char* msg = nullptr);
	[[ noreturn ]] inline void ink_assert(const char* msg = nullptr) { ink_assert(false, msg); }
#else
	[[ noreturn ]] inline void ink_fail(const char*) { check(false); throw nullptr; }
#endif

#ifdef INK_ENABLE_STL
	using ink_exception = std::runtime_error;
#else 
	// Non-STL exception class
	class ink_exception
	{
	public:
		ink_exception(const char* msg) : _msg(msg) { }

		inline const char* message() const { return _msg; }
	private:
		const char* _msg;
	};
#endif

#ifdef INK_ENABLE_STL
	template<typename ... Tys>
	using tuple = std::tuple<Tys...>;
	template<typename T>
	constexpr auto t_get = std::get<T>;
#else
	namespace tuple_internal {
		template<size_t I, typename T>
		class tuple_leaf{
		public:
			tuple_leaf() : _value() {};
			template<typename U>
			explicit tuple_leaf(U&& u) : _value(std::forward<U>(u)) {}
			T& get() { return _value; }
			const T& get() const { return _value; }
		private:
			T _value;
			tuple_leaf(const tuple_leaf& tl) = delete;
			tuple_leaf& operator=(const tuple_leaf&) = delete;
		};

		// handle indexing
		template<size_t... Is>
		struct tuple_indexes {};

		template<size_t End, size_t Start = 0, size_t... Is>
		struct make_tuple_indexes {
			using type = typename make_tuple_indexes<End, Start+1, Is..., Start>::type;
		};
		template<size_t End, size_t... Is>
		struct make_tuple_indexes<End, End, Is...> {
			using type = tuple_indexes<Is...>;
		};

		// handle types
		template<size_t I, typename T, typename ...Tys>
		struct tuple_type : tuple_type<I-1,Tys...> {};
		template<typename T, typename ...Tys>
		struct tuple_type<0, T, Tys...> {
			using type = T;
		};

		template<typename T, size_t I, typename U, typename ...Tys>
		struct type_index_imp : type_index_imp<T,I+1,Tys...> {};
		template<typename T, size_t I, typename ...Tys>
		struct type_index_imp<T,I,T,Tys...> {
			static constexpr size_t value = I;
		};
		template<typename T, typename ...Tys>
		constexpr size_t type_index = type_index_imp<T,0,Tys...>::value;


		template<typename Indexes, typename ...Tys>
		struct tuple_imp;

		template<size_t ...Is,  typename ...Tys>
		struct tuple_imp<tuple_indexes<Is...>, Tys...>
			: public tuple_leaf<Is, Tys>...
		{
			template<typename ...Us>
			tuple_imp(Us&& ... us) : tuple_leaf<Is,Tys>(std::forward<Us>(us))... {
				static_assert(sizeof...(Us) == sizeof...(Tys),
						"Tuple must be initialized with same amount of arguments"
						", then types!");
			}
		};
	}

	/// minimal tuple class, only for simple data types!
	template<typename ...Tys>
	class tuple
		: public tuple_internal::tuple_imp<
		  	typename tuple_internal::make_tuple_indexes<sizeof...(Tys)>::type,
		  	Tys... >
	{
		using base = tuple_internal::tuple_imp<
			typename tuple_internal::make_tuple_indexes<sizeof...(Tys)>::type,
			Tys...>;
		using this_type = tuple<Tys...>;
		template<size_t I>
		using element_type = typename tuple_internal::tuple_type<I, Tys...>::type;
		template<typename T>
		static constexpr size_t type_index = tuple_internal::type_index<T, Tys...>;
	public:
		template<typename ...Us>
		tuple(Us&& ... us) : base(std::forward<Us>(us)...) {}

		template<size_t I>
		friend
		inline constexpr element_type<I> const&
		t_get(const this_type& t);
		template<typename T>
		friend
		const T&
		t_get(const this_type& t);
	};

	template<size_t I, typename T>
	inline constexpr typename T::template element_type<I> const&
	t_get(const T& t) {
		return static_cast<tuple_internal::tuple_leaf<I, typename T::template element_type<I>>const&>(t).get();
	};

	template<typename T, typename Tuple>
	const T&
	t_get(const Tuple& t) {
		return t_get<Tuple::template type_index<T>>(t);
	};

	template<>
	class tuple<> {
	public:
		tuple() {}
	};
#endif

	namespace runtime::internal
	{
		struct false_type { static constexpr bool value = false; };
		struct true_type { static constexpr bool value = true; };
		template<typename T>
		struct always_false : false_type {};

		template<typename T, typename Tuple>
		struct has_type;
		template<typename T>
		struct has_type<T,tuple<>> : false_type {};
		template<typename T, typename U, typename ... Tys>
		struct has_type<T, tuple<U, Tys...>> : has_type<T,tuple<Tys...>> {};
		template<typename T, typename ... Tys>
		struct has_type<T, tuple<T, Tys...>> : true_type {};
	}

#ifdef INK_ENABLE_STL
	template<typename T>
	using optional = std::optional<T>;
	constexpr std::nullopt_t nullopt = std::nullopt;
#else
	struct nullopt_t{};
	constexpr nullopt_t nullopt;

	template<typename T>
	class optional {
	public:
		optional() {}
		optional(nullopt_t) {}
		optional(T&& val) _has_value{true}, _value{std::forward(val)}{}
		optional(const T& val) _has_value{true}, _value{val}{}

		const T& operator*() const { return _value; }
		T& operator*() { return _value; }
		const T* operator->() const { return &_value; }
		T* operator->() { return &_value; }

		constexpr bool has_value() const { return _has_value; }
		constexpr T& value() { check(); return _value; }
		constexpr const T& value() const { check(); return _value; }
		constexpr operator bool() const { return has_value(); }
		template<typename U>
		constexpr T value_or(U&& u) const {
			return _has_value ? _value : static_cast<T>(std::forward(u));
		}
	private:
		void check() const {
			if ( ! _has_value) {
				throw ink_exception("Can't access empty optional!");
			}
		}

		bool _has_value = false;
		T _value;
	};
#endif
}

// Platform specific defines //

#ifdef INK_ENABLE_UNREAL
#define inkZeroMemory(buff, len) FMemory::Memset(buff, 0, len)
#define inkAssert(condition, text) checkf(condition, TEXT(text))
#define inkFail(text) ink::ink_fail(text)
#else
#define inkZeroMemory ink::zero_memory
#define inkAssert ink::ink_assert
#define inkFail(text) ink::ink_assert(text)
#endif
