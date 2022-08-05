#include "header.h"
#include "version.h"

namespace ink::internal {

	header header::parse_header(const char *data)
	{
		header res;
		const char* ptr = data;
		res.endien = *reinterpret_cast<const header::endian_types*>(ptr);
		ptr += sizeof(header::endian_types);

		using v_t = decltype(header::ink_version_number);
		using vcpp_t = decltype(header::ink_bin_version_number);

		if (res.endien == header::endian_types::same) {
			res.ink_version_number =
				*reinterpret_cast<const v_t*>(ptr);
			ptr += sizeof(v_t);
			res.ink_bin_version_number =
				*reinterpret_cast<const vcpp_t*>(ptr);

		} else if (res.endien == header::endian_types::differ) {
			res.ink_version_number =
				swap_bytes(*reinterpret_cast<const v_t*>(ptr));
			ptr += sizeof(v_t);
			res.ink_bin_version_number =
				swap_bytes(*reinterpret_cast<const vcpp_t*>(ptr));
		} else {
			inkFail("Failed to parse endian encoding!");
		}

		if (res.ink_bin_version_number != InkBinVersion) {
			inkFail("InkCpp-version mismatch: file was compiled with different InkCpp-version!");
		}
		return res;
	}
}
