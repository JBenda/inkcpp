#include "pch.h"
#include "story.h"

namespace ink
{
	namespace runtime
	{
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
			: file(nullptr)
			, string_table(nullptr)
			, instruction_data(nullptr)
			, length(0)
		{
			// Load file into memory
			file = read_file_into_memory(filename, &length);

			// String table is after the version information
			string_table = (char*)file + sizeof(int);

			// Pass over strings
			const char* ptr = string_table;
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
			instruction_data = (ip_t)ptr + 1;
		}

		story::~story()
		{
			if (file != nullptr)
				delete[] file;

			file = nullptr;
			instruction_data = nullptr;
			string_table = nullptr;
		}


		const char* story::string(uint32_t index) const
		{
			return string_table + index;
		}
	}
}