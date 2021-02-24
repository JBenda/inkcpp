#include "choice.h"
#include "output.h"

namespace ink
{
	namespace runtime
	{
		void choice::setup(internal::basic_stream& in, internal::string_table& strings, int index, uint32_t path, thread_t thread)
		{
			// if we only have one item in our output stream
			if (in.queued() == 2)
			{
				// If it's a string, just grab it. Otherwise, use allocation
				const internal::value& data = in.peek();
				switch (data.type())
				{
				case internal::value_type::string:
					_text = data.get<internal::value_type::string>();
					in.discard(2);
					break;
				default:
					_text = in.get_alloc(strings);
				}
			}
			else
			{
				// Non-string. Must allocate
				_text = in.get_alloc(strings);
			}

			// Index/path
			_index = index;
			_path = path;
			_thread = thread;
		}
	}
}
