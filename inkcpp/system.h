#pragma once

#undef assert

namespace ink
{
	typedef unsigned int uint32_t;

	// Name hash (used for temporary variables)
	typedef uint32_t hash_t;

	// Invalid hash
	const hash_t InvalidHash = 0;

	// Simple hash for serialization of strings
	hash_t hash_string(const char* string);

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

	// Checks if a string is only whitespace
	bool is_whitespace(const char* string, bool includeNewline = true);

	// Zero memory
	void zero_memory(void* buffer, size_t length);

	// assert
	void assert(bool condition, const char* msg = nullptr);

#ifdef INK_ENABLE_STL
	using ink_exception = std::exception;
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

		template<typename T>
		struct restorable_type_null
		{
			static_assert(always_false<T>, "No restorable_type_null defined for this type!");
		};

		template<>
		struct restorable_type_null<uint32_t>
		{
			static constexpr uint32_t value = ~0;
		};

		template<typename T>
		struct restorable_type_null<T*>
		{
			static constexpr T* value = nullptr;
		};
	}
}

// Platform specific defines //

#define inkZeroMemory ink::zero_memory