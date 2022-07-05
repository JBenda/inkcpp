#pragma once

#include "types.h"

namespace ink::runtime
{
	class snapshot {
	public:
		snapshot() = delete;
		virtual ~snapshot() = 0;

		static snapshot* from_binary(unsigned char* data, size_t length, bool freeOnDestroy = true);
		static snapshot* from_file(const char* filename);

		virtual const char* get_data() const = 0;
		virtual unsigned get_data_len() const = 0;
	};
}
