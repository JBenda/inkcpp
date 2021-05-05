#pragma once

#include "system.h"

namespace ink::internal {

		struct header {
			static header parse_header(const char* data);

			template<typename T>
			static T swap_bytes(const T& value) {
				char data[sizeof(T)];
				for (int i = 0; i < sizeof(T); ++i) {
					data[i] = reinterpret_cast<const char*>(&value)[sizeof(T)-1-i];
				}
				return *reinterpret_cast<const T*>(data);
			}
			list_flag read_list_flag(const char*& ptr) const {
				list_flag result = *reinterpret_cast<const list_flag*>(ptr);
				ptr += sizeof(list_flag);
				if (endien == ink::internal::header::endian_types::differ) {
					result.flag = swap_bytes(result.flag);
					result.list_id = swap_bytes(result.list_id);
				}
				return result;
			}

			enum class  endian_types: uint16_t {
				none = 0,
				same = 0x0001,
				differ = 0x0100
			} endien = endian_types::none;
			uint32_t ink_version_number = 0;
			uint32_t ink_bin_version_number = 0;
			static constexpr size_t Size = ///< actual data size of Header,
										   ///   because padding of struct may
										   ///   differ between platforms
				sizeof(uint16_t) + 2 * sizeof(uint32_t);
		};
}
