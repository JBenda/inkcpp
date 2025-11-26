/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "system.h"
#include "traits.h"
#include "value.h"

#include <cstdio>

#ifndef EINVAL
#	define EINVAL -1
#endif

namespace ink::runtime::internal
{
// error behavior from:
// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/itoa-s-itow-s?view=msvc-160
inline int toStr(char* buffer, size_t size, uint32_t value)
{
#ifdef WIN32
	return _itoa_s(static_cast<int>(value), buffer, size, 10);
#else
	if (buffer == nullptr || size < 1) {
		return EINVAL;
	}
	int res = snprintf(buffer, size, "%d", value);
	if (res > 0 && static_cast<size_t>(res) < size) {
		return 0;
	}
	return EINVAL;
#endif
}

// error behavior from:
// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/itoa-s-itow-s?view=msvc-160
inline int toStr(char* buffer, size_t size, int32_t value)
{
#ifdef WIN32
	return _itoa_s(value, buffer, size, 10);
#else
	if (buffer == nullptr || size < 1) {
		return EINVAL;
	}
	int res = snprintf(buffer, size, "%d", value);
	if (res > 0 && static_cast<size_t>(res) < size) {
		return 0;
	}
	return EINVAL;
#endif
}

// error behavior from:
// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/gcvt-s?view=msvc-160
inline int toStr(char* buffer, size_t size, float value)
{
#ifdef WIN32
	int digits = 7;
	for (float f = value; f > 1.f; f /= 10.f) {
		++digits;
	}
	int ec = _gcvt_s(buffer, size, value, digits); // number of significant digits
#else
	if (buffer == nullptr || size < 1) {
		return EINVAL;
	}
	int res = snprintf(buffer, size, "%.7f", value);
	if (res < 0 || static_cast<size_t>(res) >= size) {
		return EINVAL;
	}
	// trunc cat zeros B007
	int ec = 0;
#endif
	char* itr = buffer + strlen(buffer) - 1;
	while (*itr == '0') {
		*itr-- = 0;
	}
	if (*itr == '.') {
		*itr-- = 0;
	}
	return ec;
}

inline int toStr(char* buffer, size_t size, const char* str)
{
	char*  ptr = buffer;
	size_t i   = 0;
	while (i < size && str[i]) {
		ptr[i] = str[i];
		++i;
	}
	if (i >= size) {
		return EINVAL;
	}
	ptr[i] = 0;
	return 0;
}

inline int toStr(char* buffer, size_t size, bool b)
{
	return toStr(buffer, size, b ? "true" : "false");
}

inline int toStr(char* buffer, size_t size, const value& v)
{
	switch (v.type()) {
		case value_type::int32: return toStr(buffer, size, v.get<value_type::int32>());
		case value_type::uint32: return toStr(buffer, size, v.get<value_type::uint32>());
		case value_type::float32: return toStr(buffer, size, v.get<value_type::float32>());
		case value_type::boolean: return toStr(buffer, size, v.get<value_type::boolean>());
		case value_type::newline: return toStr(buffer, size, "\n");
		default: inkFail("No toStr implementation for this type"); return -1;
	}
}

// return a upper bound for the string representation of the number
inline constexpr size_t decimal_digits(uint32_t number)
{
	size_t length = 1;
	while (number /= 10) {
		++length;
	}
	return length;
}

inline constexpr size_t decimal_digits(int32_t number)
{
	size_t length = number < 0 ? 2 : 1;
	while (number /= 10) {
		++length;
	}
	return length;
}

inline constexpr size_t decimal_digits(float number)
{
	return decimal_digits(static_cast<int32_t>(number)) + 8;
}

inline constexpr size_t value_length(const value& v)
{
	switch (v.type()) {
		case value_type::int32: return decimal_digits(v.get<value_type::int32>());
		case value_type::uint32: return decimal_digits(v.get<value_type::uint32>());
		case value_type::float32: return decimal_digits(v.get<value_type::float32>());
		case value_type::string: return c_str_len(v.get<value_type::string>());
		case value_type::boolean:
			return v.get<value_type::boolean>() ? c_str_len("true") : c_str_len("false");
		case value_type::newline: return 1;
		default: inkFail("Can't determine length of this value type"); return ~0U;
	}
}

inline constexpr bool str_equal(const char* lh, const char* rh)
{
	while (*lh && *rh && *lh == *rh) {
		++lh;
		++rh;
	}
	return *lh == *rh;
}

inline constexpr bool str_equal_len(const char* lh, const char* rh, size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		if (! (*rh && *lh && *lh == *rh)) {
			return false;
		}
	}
	return true;
}

inline constexpr const char* str_find(const char* str, char c)
{
	while (*str && *str != c) {
		++str;
	}
	if (*str == c) {
		return str;
	}
	return nullptr;
}

/** removes leading & tailing spaces as wide spaces
 * @param begin iterator of string
 * @param end iterator of string
 * @return new end iterator
 */
template<bool LEADING_SPACES, bool TAILING_SPACES, typename ITR>
inline constexpr ITR clean_string(ITR begin, ITR end)
{
	auto dst = begin;
	for (auto src = begin; src != end; ++src) {
		if (dst == begin) {
			if constexpr (LEADING_SPACES) {
				if (isspace(static_cast<unsigned char>(src[0]))) {
					continue;
				}
			}
		} else if (src[-1] == '\n' && isspace(static_cast<unsigned char>(src[0]))) {
			continue;
		} else if (isspace(static_cast<unsigned char>(src[0])) && src[0] != '\n') {
			if constexpr (TAILING_SPACES) {
				if (src + 1 == end) {
					continue;
				}
			}
			if (src + 1 != end && isspace(static_cast<unsigned char>(src[1]))) {
				continue;
			}
		} else if (src[0] == '\n' && dst != begin && dst[-1] == '\n') {
			continue;
		}
		*dst++ = *src;
	}
	return dst;
}
} // namespace ink::runtime::internal
