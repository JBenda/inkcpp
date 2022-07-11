#include "output.h"
#include "string_table.h"
#include "list_table.h"
#include <system.h>
#include "string_utils.h"

#ifdef INK_ENABLE_STL
#include <iomanip>
#endif

namespace ink::runtime::internal
{
	basic_stream::basic_stream(value* buffer, size_t len)
		: _data(buffer), _max(len), _size(0), _save(~0)
	{}

	void basic_stream::append(const value& in)
	{
		// SPECIAL: Incoming newline
		if (in.type() == value_type::newline && _size > 1)
		{
			// If the end of the stream is a function start marker, we actually
			//  want to ignore this. Function start trimming.
			if (_data[_size - 1].type() == value_type::func_start)
				return;
		}

		// Ignore leading newlines
		if (in.type() == value_type::newline && _size == 0)
			return;

		// Add to data stream
		inkAssert(_size < _max, "Output stream overflow");
		_data[_size++] = in;

		// Special: Incoming glue. Trim whitespace/newlines prior
		//  This also applies when a function ends to trim trailing whitespace.
		if ((in.type() == value_type::glue || in.type() == value_type::func_end) && _size > 1)
		{
			// Run backwards
			size_t i = _size - 2;
			while(true)
			{
				value& d = _data[i];

				// Nullify newlines
				if (d.type() == value_type::newline) {
					d = value{};
				}

				// Nullify whitespace
				else if ( d.type() == value_type::string 
					&& is_whitespace(d.get<value_type::string>()))
					d = value{};

				// If it's not a newline or whitespace, stop
				else break;

				// If we've hit the end, break
				if (i == 0)
					break;

				// Move on to next element
				i--;
			}
		}
	}

	void basic_stream::append(const value* in, unsigned int length)
	{
		// TODO: Better way to bulk while still executing glue checks?
		for (size_t i = 0; i < length; i++)
			append(in[i]);
	}

	template<typename OUT>
	inline void write_char(OUT& output, char c)
	{
		static_assert(always_false<OUT>::value, "Invalid output type");
	}

	template<>
	inline void write_char(char*& output, char c)
	{
		(*output++) = c;
	}

	template<>
	inline void write_char(std::stringstream& output, char c)
	{
		output.put(c);
	}

	inline bool get_next(const value* list, size_t i, size_t size, const value** next)
	{
		while (i + 1 < size)
		{
			*next = &list[i + 1];
			value_type type = (*next)->type();
			if ((*next)->printable()) { return true; }
			i++;
		}

		return false;
	}

	template<typename OUT>
	void basic_stream::copy_string(const char* str, size_t& dataIter, OUT& output)
	{
		while(*str != 0) {
		write_char(output, *str++);
		}
	}

#ifdef INK_ENABLE_STL
	std::string basic_stream::get()
	{
		size_t start = find_start();

		// Move up from marker
		bool hasGlue = false, lastNewline = false;
		std::stringstream str;
		for (size_t i = start; i < _size; i++)
		{
			if (should_skip(i, hasGlue, lastNewline))
				continue;
			if (_data[i].printable()){
				_data[i].write(str, _lists_table);
			}

		}

		// Reset stream size to where we last held the marker
		_size = start;

		// Return processed string
		// remove mulitple accourencies of ' '
		std::string result = str.str();
		auto end = clean_string<true, false>(result.begin(), result.end());
		_last_char = *(end-1);
		result.resize(end - result.begin() - (_last_char == ' ' ? 1 : 0));
		return result;
	}
#endif
#ifdef INK_ENABLE_UNREAL
	FString basic_stream::get()
	{
		size_t start = find_start();

		// TODO: Slow! FString concatonation.
		//  Is there really no equivilent of stringstream in Unreal? Some kind of String Builder?

		// Move up from marker
		bool hasGlue = false;
		FString str;
		for (size_t i = start; i < _size; i++)
		{
			if (should_skip(i, hasGlue))
				continue;

			switch (_data[i].type)
			{
			case value_type::int32:
				str += FString::Printf(TEXT("%d"), _data[i].integer_value);
				break;
			case value_type::float32:
				// TODO: Whitespace cleaning
				str += FString::Printf(TEXT("%f"), _data[i].float_value);
				break;
			case value_type::string:
				str += _data[i].string_val;
				break;
			case data_type::newline:
				str += "\n";
				break;
			default:
				break;
			}
		}

		// Reset stream size to where we last held the marker
		_size = start;

		// Return processed string
		return str;
	}
#endif

	int basic_stream::queued() const
	{
		size_t start = find_start();
		return _size - start;
	}

	const value& basic_stream::peek() const
	{
		inkAssert(_size > 0, "Attempting to peek empty stream!");
		return _data[_size - 1];
	}

	void basic_stream::discard(size_t length)
	{
		// discard elements
		_size -= length;
		if (_size < 0)
			_size = 0;
	}

	void basic_stream::get(value* ptr, size_t length)
	{
		// Find start
		size_t start = find_start();

		const value* end = ptr + length;
		//inkAssert(_size - start < length, "Insufficient space in data array to store stream contents!");

		// Move up from marker
		bool hasGlue = false, lastNewline = false;
		for (size_t i = start; i < _size; i++)
		{
			if (should_skip(i, hasGlue, lastNewline))
				continue;

			// Make sure we can fit the next element
			inkAssert(ptr < end, "Insufficient space in data array to store stream contents!");

			// Copy any value elements
			if (_data[i].printable()) {
				*(ptr++) = _data[i];
			}
		}

		// Reset stream size to where we last held the marker
		_size = start;
	}

	bool basic_stream::has_marker() const
	{
		// TODO: Cache?
		for (size_t i = 0; i < _size; i++)
		{
			if (_data[i].type() == value_type::marker)
				return true;
		}

		return false;
	}

	bool basic_stream::ends_with(value_type type) const
	{
		if (_size == 0)
			return false;

		return _data[_size - 1].type() == type;
	}

	bool basic_stream::saved_ends_with(value_type type) const
	{
		inkAssert(_save != ~0, "Stream is not saved!");

		if (_save == 0)
			return false;

		return _data[_save - 1].type() == type;
	}

	void basic_stream::save()
	{
		inkAssert(_save == ~0, "Can not save over existing save point!");

		// Save the current size
		_save = _size;
	}

	void basic_stream::restore()
	{
		inkAssert(_save != ~0, "No save point to restore!");

		// Restore size to saved position
		_size = _save;
		_save = ~0;
	}

	void basic_stream::forget()
	{
		inkAssert(_save != ~0, "No save point to forget!");

		// Just null the save point and continue as normal
		_save = ~0;
	}
	
	template char* basic_stream::get_alloc<true>(string_table& strings, list_table& lists);
	template char* basic_stream::get_alloc<false>(string_table& strings, list_table& lists);

	template<bool RemoveTail>
	char* basic_stream::get_alloc(string_table& strings, list_table& lists)
	{
		size_t start = find_start();

		// Two passes. First for length
		size_t length = 0;
		bool hasGlue = false, lastNewline = false;
		for (size_t i = start; i < _size; i++)
		{
			if (should_skip(i, hasGlue, lastNewline))
				continue;
			++length; // potenzial space to sperate 
			if (_data[i].printable()) {
				switch(_data[i].type()) {
					case value_type::list:
						length += lists.stringLen(_data[i].get<value_type::list>());
						break;
					case value_type::list_flag:
						length += lists.stringLen(_data[i].get<value_type::list_flag>());
					default: length += value_length(_data[i]);
				}
			}
		}

		// Allocate
		char* buffer = strings.create(length + 1);
		char* end = buffer + length + 1;
		char* ptr = buffer;
		hasGlue = false; lastNewline = false;
		for (size_t i = start; i < _size; i++)
		{
			if (should_skip(i, hasGlue, lastNewline))
				continue;
			if(!_data[i].printable()) { continue; }
			switch (_data[i].type())
			{
			case value_type::int32:
			case value_type::float32:
			case value_type::uint32:
				// Convert to string and advance
				toStr(ptr, end - ptr, _data[i]);
				while (*ptr != 0) ptr++;

				break;
			case value_type::string:
			{
				// Copy string and advance
				const char* value = _data[i].get<value_type::string>(); 
				copy_string(value, i, ptr);
			}	break;
			case value_type::newline:
				*ptr = '\n'; ptr++;
				break;
			case value_type::list:
				ptr = lists.toString(ptr, _data[i].get<value_type::list>());
				break;
			case value_type::list_flag:
				ptr = lists.toString(ptr, _data[i].get<value_type::list>());
				break;
			default: throw ink_exception("cant convert expression to string!");
			}
		}

		// Make sure last character is a null
		*ptr = 0;

		// Reset stream size to where we last held the marker
		_size = start;

		// Return processed string
		{
		 auto end = clean_string<false,false>(buffer, buffer+length);
		 *end = 0;
		 _last_char = end[-1];
		 if constexpr (RemoveTail) {
			 if (_last_char == ' ') { end[-1] = 0; }
		 }
		}
		return buffer;
	}

	size_t basic_stream::find_start() const
	{
		// Find marker (or start)
		size_t start = _size;
		while (start > 0)
		{
			start--;
			if (_data[start].type() == value_type::marker)
				break;
		}

		// Make sure we're not violating a save point
		if (_save != ~0 && start < _save) {
			// TODO: check if we don't reset save correct
			// at some point we can modifiy the output even behind save (probally discard?) and push a new element -> invalid save point
			// inkAssert(false, "Trying to access output stream prior to save point!");
			const_cast<basic_stream&>(*this).clear();
		}

		return start;
	}

	bool basic_stream::should_skip(size_t iter, bool& hasGlue, bool& lastNewline) const
	{
		if (_data[iter].printable()
				&& _data[iter].type() != value_type::newline
				&& _data[iter].type() != value_type::string) {
			lastNewline = false;
			hasGlue = false; 
		} else {
		switch (_data[iter].type())
		{
			case value_type::newline:
				if (lastNewline)
					return true;
				if (hasGlue)
					return true;
				lastNewline = true;
				break;
			case value_type::glue:
				hasGlue = true;
				break;
			case value_type::string:
			{
				lastNewline = false;
				// an empty string don't count as glued I095
				for(const char* i=_data[iter].get<value_type::string>();
						*i; ++i)
				{
					// isspace only supports characters in [0, UCHAR_MAX]
					if (!isspace(static_cast<unsigned char>(*i))) {
						hasGlue = false;
						break;
					}
				}
			} break;
			default:
				break;
			}
		}

		return false;
	}

	bool basic_stream::text_past_save() const
	{
		// Check if there is text past the save
		for (size_t i = _save; i < _size; i++)
		{
			const value& d = _data[i];
			if (d.type() == value_type::string)
			{
				// TODO: Cache what counts as whitespace?
				if (!is_whitespace(d.get<value_type::string>(), false))
					return true;
			}
		}

		// No text
		return false;
	}

	void basic_stream::clear()
	{
		_save = ~0;
		_size = 0;
	}

	void basic_stream::mark_strings(string_table& strings) const
	{
		// Find all allocated strings and mark them as used
		for (int i = 0; i < _size; i++)
		{
			if (_data[i].type() == value_type::string) {
				string_type str = _data[i].get<value_type::string>();
				if (str.allocated) {
					strings.mark_used(str.str);
				}
			}
		}
	}

#ifdef INK_ENABLE_STL
	std::ostream& operator<<(std::ostream& out, basic_stream& in)
	{
		out << in.get();
		return out;
	}

	basic_stream& operator>>(basic_stream& in, std::string& out)
	{
		out = in.get();
		return in;
	}
#endif
#ifdef INK_ENABLE_UNREAL
	basic_stream& operator>>(basic_stream& in, FString& out)
	{
		out = in.get();
		return in;
	}
#endif

	size_t basic_stream::snap(unsigned char* data, const snapper& snapper) const
	{
		unsigned char* ptr = data;
		ptr = snap_write(ptr, _last_char, data);
		ptr = snap_write(ptr, _size, data);
		ptr = snap_write(ptr, _save, data);
		for(auto itr = _data; itr != _data + _size; ++itr)
		{
			ptr += itr->snap(data ? ptr : nullptr, snapper);
		}
		return ptr - data;
	}

	const unsigned char* basic_stream::snap_load(const unsigned char* ptr, const loader& loader)
	{
		ptr = snap_read(ptr, _last_char);
		ptr = snap_read(ptr, _size);
		ptr = snap_read(ptr, _save);
		inkAssert(_max >= _size, "output is to small to hold stored data");
		for(auto itr = _data; itr != _data + _size; ++itr)
		{
			ptr = itr->snap_load(ptr, loader);
		}
		return ptr;
	}
}
