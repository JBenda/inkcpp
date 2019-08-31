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

	// Checks if a string is only whitespace
	bool is_whitespace(const char* string, bool includeNewline = true);

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
}
