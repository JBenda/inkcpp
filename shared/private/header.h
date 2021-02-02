#pragma once

#include "system.h"

namespace ink {
		struct Header {
			static Header parse_header(const char* data);

			template<typename T>
			static T swap_bytes(const T& value) {
				char data[sizeof(T)];
				for (int i = 0; i < sizeof(T); ++i) {
					data[i] = reinterpret_cast<const char*>(&value)[sizeof(T)-1-i];
				}
				return *reinterpret_cast<const T*>(data);
			}

			enum class ENDENSE : uint16_t {
				NONE = 0,
				SAME = 0x0001,
				DIFFER = 0x0100
			} endien = ENDENSE::NONE;
			uint32_t inkVersionNumber = 0;
			uint32_t inkCppVersionNumber = 0;
			static constexpr size_t SIZE = ///< actual data size of Header,
										   ///   because padding of struct may
										   ///   differ between platforms
				sizeof(uint16_t) + 2 * sizeof(uint32_t);
		};
}
