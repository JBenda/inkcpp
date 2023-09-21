#include "choice.h"

#include "output.h"
#include "string_table.h"
#include "string_utils.h"

namespace ink {
namespace runtime {
	choice& choice::setup( internal::basic_stream& in, internal::string_table& strings, internal::list_table& lists, int index, uint32_t path, thread_t thread, const char* const* tags )
	{
		char* text = nullptr;
		// if we only have one item in our output stream
		if ( in.queued() == 2 )
		{
			// If it's a string, just grab it. Otherwise, use allocation
			const internal::value& data = in.peek();
			switch ( data.type() )
			{
			case internal::value_type::string:
				text = strings.duplicate( data.get<internal::value_type::string>() );
				in.discard( 2 );
				break;
			default:
				text = in.get_alloc( strings, lists );
			}
		}
		else
		{
			// Non-string. Must allocate
			text = in.get_alloc( strings, lists );
		}
		char* end = text;
		while ( *end )
		{
			++end;
		}
		end   = ink::runtime::internal::clean_string<true, true>( text, end );
		*end  = 0;
		_text = text;
		// Index/path
		_index  = index;
		_path   = path;
		_thread = thread;
		_tags   = tags;
		return *this;
	}
}
}   // namespace ink::runtime
