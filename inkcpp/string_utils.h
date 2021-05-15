#pragma once

#include "system.h"

#include <cstdio>

namespace ink::runtime::internal {

	// error behavior from: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/itoa-s-itow-s?view=msvc-160
	inline int toStr(char * buffer, size_t size, uint32_t value) {
#ifdef WIN32
		return _itoa_s(value, buffer, size, 10);
#else
		if ( buffer == nullptr || size < 1 ){ return EINVAL; }
		int res = snprintf(buffer, size, "%d", value);
		if (res > 0 && res < size) { return 0; }
		return EINVAL;
#endif
	}

	// error behavior from: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/itoa-s-itow-s?view=msvc-160
	inline int toStr(char * buffer, size_t size, int32_t value) {
#ifdef WIN32
		return _itoa_s(value, buffer, size, 10);
#else
		if ( buffer == nullptr || size < 1 ){ return EINVAL; }
		int res = snprintf(buffer, size, "%d", value);
		if (res > 0 && res < size) { return 0; }
		return EINVAL;
#endif
	}

	// error behavior from: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/gcvt-s?view=msvc-160
	inline int toStr(char * buffer, size_t size, float value) {
#ifdef WIN32
		return _gcvt_s(buffer, size, value, 7); // number of significant digits
#else
		if ( buffer == nullptr || size < 1 ) { return EINVAL; }
		int res = snprintf(buffer, size, "%f.7", value);
		if (res > 0 && res < size) { return 0; }
		return EINVAL;
#endif
	}
	inline size_t strlen(const char* str) {
		size_t len = 0;
		for(const char* c = str; *c; ++c) {
			++len;
		}
		return len;
	}

	// return a upper bound for the string representation of the number
	inline constexpr size_t decimal_digits(uint32_t number) {
		size_t length = 1;
		while(number /= 10) { ++length; }
		return length;
	}

	inline constexpr size_t decimal_digits(int32_t number) {
		size_t length = number < 0 ? 2 : 1;
		while(number /= 10) { ++length; }
		return length;
	}

	inline constexpr size_t decimal_digits(float number) {
		return 16;
	}
}
