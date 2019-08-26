#include "pch.h"

#include "system.h"

namespace binary
{
	namespace system
	{
#define A 54059 /* a prime */
#define B 76963 /* another prime */
#define C 86969 /* yet another prime */
#define FIRSTH 37 /* also prime */

		NameHash hash_string(const char* s)
		{
			NameHash h = FIRSTH;
			while (*s) {
				h = (h * A) ^ (s[0] * B);
				s++;
			}
			return h; // or return h % C;
		}
	}
}