#pragma once

namespace binary
{
	namespace system
	{
		typedef uint32_t NameHash;

		// Simple hash for serialization of strings
		NameHash hash_string(const char* string);
	}
}
