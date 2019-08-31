#pragma once

#include "system.h"
#include "config.h"

namespace ink
{
	namespace runtime
	{
		// Ink story. Constant once constructed. Can be shared safely between multiple runner instances
		class story
		{
		public:
#ifdef INK_ENABLE_STL
			story(const char* filename);
#endif
			// Create story from allocated binary data in memory. If manage is true, this class will delete
			//  the pointers on destruction
			story(unsigned char* binary, size_t len, bool manage = true);
			~story();

			const char* string(uint32_t index) const;
			inline const ip_t instructions() const { return _instruction_data; }
			inline const ip_t end() const { return _file + _length; }

		private:
			void setup_pointers();

		private:
			unsigned char* _file;
			size_t _length;
			const char* _string_table;
			ip_t _instruction_data;
			bool _managed;
		};
	}
}