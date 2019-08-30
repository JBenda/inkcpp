#pragma once

#include "system.h"

namespace ink
{
	namespace runtime
	{
		// Ink story. Constant once constructed. Can be shared safely between multiple runner instances
		class story
		{
		public:
			story(const char* filename);
			~story();

			const char* string(uint32_t index) const;
			const ip_t instructions() const { return instruction_data; }
			const ip_t end() const { return file + length; }

		private:
			unsigned char* file;
			size_t length;
			const char* string_table;
			ip_t instruction_data;
		};
	}
}