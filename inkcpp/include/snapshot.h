#pragma once

#include "types.h"

namespace ink::runtime
{
	class snapshot {
	public:
		virtual ~snapshot() {};

		static snapshot* from_binary(const unsigned char* data, size_t length, bool freeOnDestroy = true);

		virtual const unsigned char* get_data() const = 0;
		virtual size_t get_data_len() const = 0;
		virtual size_t num_runners() const = 0;

#ifdef INK_ENABLE_STL
		static snapshot* from_file(const char* filename);
		void write_to_file(const char* filename) const;
#endif
	};
}
