/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "system.h"

#include <cstring>

namespace ink::runtime::internal
{
class globals_impl;
template<typename, bool, size_t, bool = false>
class managed_array;
class snap_tag;
class string_table;
class value;

class snapshot_interface
{
public:
	constexpr snapshot_interface(){};

	static unsigned char* snap_write(unsigned char* ptr, const void* data, size_t length, bool write)
	{
		if (write) {
			memcpy(ptr, data, length);
		}
		return ptr + length;
	}

	template<typename T>
	static unsigned char* snap_write(unsigned char* ptr, const T& data, bool write)
	{
		return snap_write(ptr, &data, sizeof(data), write);
	}

	static const unsigned char* snap_read(const unsigned char* ptr, void* data, size_t length)
	{
		memcpy(data, ptr, length);
		return ptr + length;
	}

	template<typename T>
	static const unsigned char* snap_read(const unsigned char* ptr, T& data)
	{
		return snap_read(ptr, &data, sizeof(data));
	}

	struct snapper {
		const string_table& strings;
		const char*         story_string_table;
		const snap_tag*     runner_tags = nullptr;

		snapper(const string_table& strings, const char* story_string_table)
		    : strings{strings}
		    , story_string_table{story_string_table}
		{
		}

		snapper()                          = delete;
		snapper& operator=(const snapper&) = delete;
	};

	struct loader {
		managed_array<const char*, true, 5>& string_table; /// FIXME: make configurable
		const char*                          story_string_table;
		const snap_tag*                      runner_tags = nullptr;

		loader(managed_array<const char*, true, 5>& string_table, const char* story_sting_table)
		    : string_table{string_table}
		    , story_string_table{story_string_table}
		{
		}

		loader()                         = delete;
		loader& operator=(const loader&) = delete;
	};

#ifdef __GNUC__
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-parameter"
#else
#	pragma warning(push)
#	pragma warning(                                                                          \
	    disable : 4100, justification : "non functional prototypes do not need the argument." \
	)
#endif

	size_t snap(unsigned char* data, snapper&) const
	{
		inkFail("Snap function not implemented");
		return 0;
	};

	const unsigned char* snap_load(const unsigned char* data, loader&)
	{
		inkFail("Snap function not implemented");
		return nullptr;
	};

#ifdef __GNUC__
#	pragma GCC diagnostic pop
#else
#	pragma warning(pop)
#endif
};
} // namespace ink::runtime::internal
