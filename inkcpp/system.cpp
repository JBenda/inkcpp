#include "system.h"

#ifndef INK_ENABLE_UNREAL

namespace ink
{
#define A 54059 /* a prime */
#define B 76963 /* another prime */
#define C 86969 /* yet another prime */
#define FIRSTH 37 /* also prime */

	hash_t hash_string(const char* string)
	{
		hash_t h = FIRSTH;
		while (*string) {
			h = (h * A) ^ (string[0] * B);
			string++;
		}
		return h; // or return h % C;
	}

	void zero_memory(void* buffer, size_t length)
	{
		char* buf = static_cast<char*>(buffer);
		for (size_t i = 0; i < length; i++)
			*(buf++) = 0;
	}
}

#endif