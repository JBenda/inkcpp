#include "system.h"

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

	bool is_whitespace(const char* string, bool includeNewline)
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

	void assert(bool condition, const char* msg /*= nullptr*/)
	{
		if (!condition)
			throw ink_exception(msg);
	}

}