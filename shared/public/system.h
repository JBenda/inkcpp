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
#endif

#undef assert

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
	void assert(bool condition, const char* msg = nullptr);
	[[ noreturn ]] inline void assert(const char* msg = nullptr) { assert(false, msg); }
#else
	[[ noreturn ]] inline void fail(const char*) { check(false); throw nullptr; }
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

	namespace runtime::internal
	{
		template<typename T>
		struct always_false { static constexpr bool value = false; };
	}
}

// Platform specific defines //

#ifdef INK_ENABLE_UNREAL
#define inkZeroMemory(buff, len) FMemory::Memset(buff, 0, len)
#define inkAssert(condition, text) checkf(condition, TEXT(text))
#define inkFail(text) ink::fail(text)
#else
#define inkZeroMemory ink::zero_memory
#define inkAssert ink::assert
#define inkFail(text) ink::assert(text)
#endif
