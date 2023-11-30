#pragma once

#include "snapshot.h"
#include <cstring>

namespace ink::runtime::internal
{
	class globals_impl;
	class value;
	class string_table;
	template<typename,bool,size_t>
	class managed_array;

	class snapshot_interface {
	public:
		constexpr snapshot_interface(){};
		static unsigned char* snap_write(unsigned char* ptr, const void* data, size_t length, bool write)
		{
			if (write) { memcpy(ptr, data, length); }
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
			const char* story_string_table;
			const char*const* current_runner_tags;
		};
		struct loader {
			managed_array<const char*, true, 5>& string_table; /// FIXME: make configurable
			const char* story_string_table;
			const char*const* current_runner_tags;
		};
		size_t snap(unsigned char* data, snapper&) const { inkFail("Snap function not implemented"); return 0; };
		const unsigned char* snap_load(const unsigned char* data, loader&) { inkFail("Snap function not implemented"); return nullptr;};
	};
}
