#pragma once

#include "system.h"
#include "traits.h"
#include "value.h"

#include <cstdio>

#ifndef EINVAL
#define EINVAL -1
#endif

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
		int res = snprintf(buffer, size, "%.7f", value);
		if (res < 0 || res >= size) { return EINVAL; }
		// trunc cat zeros B007
		char* itr = buffer + res - 1;
		while(*itr == '0') { *itr--=0; --res; }
		if (*itr == '.') { *itr-- = 0; --res;}
		return 0;
#endif
	}

	inline int toStr(char* buffer, size_t size, const char* c) {
		char* ptr = buffer;
		size_t i = 0;
		while(*c && i < size) {
			*ptr++ = *c;
			++i;
		}
		if (i >= size) { return EINVAL; }
		*ptr = 0;
		return 0;
	}

	inline int toStr(char * buffer, size_t size, const value& v) {
		switch(v.type()) {
			case value_type::int32:
				return toStr(buffer, size, v.get<value_type::int32>());
			case value_type::uint32:
				return toStr(buffer, size, v.get<value_type::uint32>());
			case value_type::float32:
				return toStr(buffer, size, v.get<value_type::float32>());
			case value_type::newline:
				return toStr(buffer, size, "\n");
			default:
				inkFail("only support toStr for numeric types");
			return -1;
		}
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
		return decimal_digits(static_cast<int32_t>(number)) + 8;
	}

	inline constexpr size_t value_length(const value& v) {
		switch(v.type()) {
			case value_type::int32:
				return decimal_digits(v.get<value_type::int32>());
			case value_type::uint32:
				return decimal_digits(v.get<value_type::uint32>());
			case value_type::float32:
				return decimal_digits(v.get<value_type::float32>());
			case value_type::string:
				return c_str_len(v.get<value_type::string>());
			case value_type::newline:
				return 1;
			default:
				inkFail("Can't determine length of this value type");
				return -1;
		}
	}

	inline constexpr bool str_equal(const char* lh, const char* rh) {
		while(*lh && *rh && *lh == *rh) { ++lh; ++rh; }
		return *lh == *rh;
	}

	/** removes leading & tailing spaces as wide spaces
	 * @param begin iterator of string
	 * @param end iterator of string
	 * @return new end iterator
	 */
	template<bool LEADING_SPACES, bool TAILING_SPACES, typename ITR>
	inline constexpr ITR clean_string(ITR begin, ITR end) {
		auto dst = begin;
		for(auto src = begin; src != end; ++src){
			if (dst == begin) {
				if (LEADING_SPACES && (src[0] == ' ' || src[0] == '\n')) { continue; }
			}
			else if(src[-1] == '\n' && (src[0] == ' ' || src[0] == '\n')) { continue;}
			else if ( src[0] == ' ' && ( ( src + 1 == end && TAILING_SPACES ) || (( src + 1 != end ) && ( src[1] == ' ' || src[1] == '\n' ) ) ) )
			{
				continue;
			}
			else if(src[0] == '\n' && dst != begin && dst[-1] == '\n') { continue; }
			*dst++ = *src;
		}
		return dst;
	}
}
