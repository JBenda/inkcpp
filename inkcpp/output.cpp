#include "output.h"
#include "string_table.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			basic_stream::basic_stream(data* buffer, size_t len)
				: _data(buffer), _max(len), _size(0), _save(~0)
			{

			}

			void basic_stream::append(const data& in)
			{
				// TODO: CHECK SIZE
				_data[_size++] = in;

				// Special: Incoming glue. Trim whitespace/newlines prior
				if (in.type == data_type::glue && _size > 1)
				{
					// Run backwards
					for (size_t i = _size - 2; i >= 0; i--)
					{
						data& d = _data[i];

						// Nullify newlines
						if (d.type == data_type::newline)
							d.type = data_type::none;

						// Nullify whitespace
						else if (
							(d.type == data_type::string_table_pointer || d.type == data_type::allocated_string_pointer)
							&& is_whitespace(d.string_val))
							d.type = data_type::none;

						// If it's not a newline or whitespace, stop
						else break;
					}
				}
			}

			void basic_stream::append(const data* in, unsigned int length)
			{
				// TODO: Better way to bulk while still executing glue checks?
				for (size_t i = 0; i < length; i++)
					append(in[i]);
			}

#ifdef INK_ENABLE_STL
			std::string basic_stream::get()
			{
				size_t start = find_start();

				// Move up from marker
				bool hasGlue = false;
				std::stringstream str;
				for (size_t i = start; i < _size; i++)
				{
					if (should_skip(i, hasGlue))
						continue;

					switch (_data[i].type)
					{
					case data_type::int32:
						str << _data[i].integer_value;
						break;
					case data_type::float32:
						str << _data[i].float_value;
						break;
					case data_type::string_table_pointer:
					case data_type::allocated_string_pointer:
						str << _data[i].string_val;
						break;
					case data_type::newline:
						str << std::endl;
						break;
					}
				}

				// Reset stream size to where we last held the marker
				_size = start;

				// Return processed string
				return str.str();
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
					case data_type::int32:
						str += FString::Printf(TEXT("%d"), _data[i].integer_value);
						break;
					case data_type::float32:
						str += FString::Printf(TEXT("%f"), _data[i].float_value);
						break;
					case data_type::string_table_pointer:
					case data_type::allocated_string_pointer:
						str += _data[i].string_val;
						break;
					case data_type::newline:
						str += "\n";
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

			const data& basic_stream::peek() const
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

			void basic_stream::get(data* ptr, size_t length)
			{
				// Find start
				size_t start = find_start();

				const data* end = ptr + length;
				//inkAssert(_size - start < length, "Insufficient space in data array to store stream contents!");

				// Move up from marker
				bool hasGlue = false;
				for (size_t i = start; i < _size; i++)
				{
					if (should_skip(i, hasGlue))
						continue;

					// Make sure we can fit the next element
					inkAssert(ptr < end, "Insufficient space in data array to store stream contents!");

					// Copy any value elements
					switch (_data[i].type)
					{
					case data_type::int32:
					case data_type::float32:
					case data_type::string_table_pointer:
					case data_type::allocated_string_pointer:
					case data_type::newline:
						*(ptr++) = _data[i];
						break;
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
					if (_data[i].type == data_type::marker)
						return true;
				}

				return false;
			}

			bool basic_stream::ends_with(data_type type) const
			{
				if (_size == 0)
					return false;

				return _data[_size - 1].type == type;
			}

			bool basic_stream::saved_ends_with(data_type type) const
			{
				inkAssert(_save != ~0, "Stream is not saved!");

				if (_save == 0)
					return false;

				return _data[_save - 1].type == type;
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

			// TODO: This uses C STD library methods. Move to my own.
//#ifdef INK_ENABLE_CSTD
			const char* basic_stream::get_alloc(string_table& strings)
			{
				size_t start = find_start();

				// Two passes. First for length
				size_t length = 0;
				bool hasGlue = false;
				for (size_t i = start; i < _size; i++)
				{
					if (should_skip(i, hasGlue))
						continue;

					switch (_data[i].type)
					{
					case data_type::int32:
						length += 11;
						break;
					case data_type::float32:
						length += 11; // ???
						break;
					case data_type::string_table_pointer:
					case data_type::allocated_string_pointer:
						length += strlen(_data[i].string_val);
						break;
					case data_type::newline:
						length += 1;
						break;
					}
				}

				// Allocate
				char* buffer = strings.create(length + 1);
				char* end = buffer + length + 1;
				char* ptr = buffer;
				hasGlue = false;
				for (size_t i = start; i < _size; i++)
				{
					if (should_skip(i, hasGlue))
						continue;

					switch (_data[i].type)
					{
					case data_type::int32:
						// Convert to string and advance
						_itoa_s(_data[i].integer_value, ptr, end - ptr, 10);
						while (*ptr != 0) ptr++;

						break;
					case data_type::float32:
						// Convert to string and advance
						_gcvt_s(ptr, end - ptr, (double)_data[i].float_value, 11);
						while (*ptr != 0) ptr++;

						break;
					case data_type::string_table_pointer:
					case data_type::allocated_string_pointer:
						// Copy string and advance
						strcpy_s(ptr, end - ptr, _data[i].string_val);
						while (*ptr != 0) ptr++;

						break;
					case data_type::newline:
						*ptr = '\n'; ptr++;
						break;
					}
				}

				// Make sure last character is a null
				*ptr = 0;

				// Reset stream size to where we last held the marker
				_size = start;

				// Return processed string
				return buffer;
			}
//#endif

			size_t basic_stream::find_start() const
			{
				// Find marker (or start)
				size_t start = _size;
				while (start > 0)
				{
					start--;
					if (_data[start].type == data_type::marker)
						break;
				}

				// Make sure we're not violating a save point
				if (_save != ~0 && start < _save)
					inkAssert(false, "Trying to access output stream prior to save point!");

				return start;
			}

			bool basic_stream::should_skip(size_t iter, bool& hasGlue) const
			{
				switch (_data[iter].type)
				{
				case data_type::int32:
				case data_type::float32:
				case data_type::string_table_pointer:
				case data_type::allocated_string_pointer:
					hasGlue = false;
					break;
				case data_type::newline:
					if (hasGlue)
						return true;
				case data_type::glue:
					hasGlue = true;
					break;
				}

				return false;
			}

			bool basic_stream::text_past_save() const
			{
				// Check if there is text past the save
				for (size_t i = _save; i < _size; i++)
				{
					const data& d = _data[i];
					if (d.type == data_type::allocated_string_pointer || d.type == data_type::string_table_pointer)
					{
						// TODO: Cache what counts as whitespace?
						if (!is_whitespace(d.string_val, false))
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
					if (_data[i].type == internal::data_type::allocated_string_pointer)
						strings.mark_used(_data[i].string_val);
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

			basic_stream& operator<<(basic_stream& out, const data& in)
			{
				out.append(in);
				return out;
			}

		}
	}
}