/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
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

  namespace internal
  {
	  void zero_memory(void* buffer, size_t length)
	  {
		  char* buf = static_cast<char*>(buffer);
		  for (size_t i = 0; i < length; i++)
			  *(buf++) = 0;
	  }
  } // namespace internal
  } // namespace ink

#endif
