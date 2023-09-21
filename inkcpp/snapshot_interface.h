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
		};
		struct loader {
			managed_array<const char*, true, 5>& string_table;
			const char* story_string_table;
		};
		virtual size_t snap(unsigned char* data, const snapper&) const = 0;
		virtual const unsigned char* snap_load(const unsigned char* data, const loader&) = 0;
	};
}
