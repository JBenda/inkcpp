/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "choice.h"

#include "output.h"
#include "snapshot_impl.h"
#include "string_table.h"
#include "string_utils.h"

namespace ink
{
namespace runtime
{

	size_t choice::num_tags() const
	{
		return static_cast<size_t>(std::distance(_tags_start, _tags_end));
	}

	const char* choice::get_tag(size_t index) const
	{
		return (index < num_tags()) ? _tags_start[index].text() : nullptr;
	}

	choice& choice::setup(
	    internal::basic_stream& in, internal::string_table& strings, internal::list_table& lists,
	    int index, uint32_t path, thread_t thread, const internal::snap_tag* tags_start,
	    const internal::snap_tag* tags_end
	)
	{
		// Index/path
		_index      = index;
		_path       = path;
		_thread     = thread;
		_tags_start = tags_start;
		_tags_end   = tags_end;

		char* text = nullptr;
		// if we only have one item in our output stream
		if (in.queued() == 2) {
			// If it's a string, just grab it. Otherwise, use allocation
			const internal::value& data = in.peek();
			switch (data.type()) {
				case internal::value_type::string:
					text = strings.duplicate(data.get<internal::value_type::string>());
					in.discard(2);
					break;
				default: text = in.get_alloc(strings, lists);
			}
		} else {
			// Non-string. Must allocate
			text = in.get_alloc(strings, lists);
		}

		char* end = text;
		while (*end != '\0') {
			++end;
		}
		end  = ink::runtime::internal::clean_string<true, true>(text, end);
		*end = '\0';

		_text = text;

		return *this;
	}
} // namespace runtime
} // namespace ink
