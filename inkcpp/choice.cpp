#include "choice.h"
#include "output.h"

namespace ink
{
	namespace runtime
	{
		void choice::setup(internal::basic_stream& in, int index, uint32_t path)
		{
			// Read stream
			internal::data out;
			in.get(&out, 1);

			// Read into text
			inkAssert(out.type == internal::data_type::string_table_pointer, "TODO: Choices only support single strings from table atm");
			_text = out.string_val;

			// Index/path
			_index = index;
			_path = path;
		}
	}
}