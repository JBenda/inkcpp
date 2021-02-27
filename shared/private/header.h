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

			enum class  endian_types: uint16_t {
				none = 0,
				same = 0x0001,
				differ = 0x0100
			} endien = endian_types::none;
			uint32_t ink_version_number = 0;
			uint32_t ink_bin_version_number = 0;
			uint32_t num_strings = 0; ///< number of strings needed for story
			static constexpr size_t Size = ///< actual data size of Header,
										   ///   because padding of struct may
										   ///   differ between platforms
				sizeof(uint16_t) + 3 * sizeof(uint32_t);
		};
}
