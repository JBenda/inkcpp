#pragma once

#include "config.h"

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

	namespace runtime::internal {
		template<typename T>
		struct always_false {
			static constexpr bool value = false;
		};
		template<bool con, typename T>
		struct enable_if {};
		template<typename T>
		struct enable_if<true, T> { using type = T; };
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
