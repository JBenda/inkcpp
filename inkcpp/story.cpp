#include "story.h"
#include "platform.h"

namespace ink
{
	namespace runtime
	{
#ifdef INK_ENABLE_STL
		unsigned char* read_file_into_memory(const char* filename, size_t* read)
		{
			using namespace std;

			ifstream ifs(filename, ios::binary | ios::ate);
			ifstream::pos_type pos = ifs.tellg();
			size_t length = (size_t)pos;
			unsigned char* data = new unsigned char[length];
			ifs.seekg(0, ios::beg);
			ifs.read((char*)data, length);
			ifs.close();

			*read = (size_t)length;
			return data;
		}

		story::story(const char* filename)
			: _file(nullptr)
			, _string_table(nullptr)
			, _instruction_data(nullptr)
			, _length(0)
			, _managed(true)
		{
			// Load file into memory
			_file = read_file_into_memory(filename, &_length);

			// Find all the right data sections
			setup_pointers();
		}
#endif

		story::story(unsigned char* binary, size_t len, bool manage /*= true*/)
			: _file(binary), _length(len), _managed(manage)
		{
			// Setup data section pointers
			setup_pointers();
		}

		story::~story()
		{
			if (_file != nullptr && _managed)
				delete[] _file;

			_file = nullptr;
			_instruction_data = nullptr;
			_string_table = nullptr;
		}

		const char* story::string(uint32_t index) const
		{
			return _string_table + index;
		}

		void story::setup_pointers()
		{
			// String table is after the version information
			_string_table = (char*)_file + sizeof(int);

			// Pass over strings
			const char* ptr = _string_table;
			while (true)
			{
				// Read until null terminator
				while (*ptr != 0)
					ptr++;

				// Check next character
				ptr++;

				// Second null. Strings are done.
				if (*ptr == 0)
					break;
			}

			// After strings comes instruction data
			_instruction_data = (ip_t)ptr + 1;
		}
	}
}