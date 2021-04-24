#include "value.h"
#include "output.h"
#include "string_table.h"
#include "string_utils.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			value::value()
			{
				_data[0].set_void();
				for (size_t i = 1; i < VALUE_DATA_LENGTH; i++)
					_data[i].set_none();
			}

			value::value(int val) : value()
			{
				_first.set_int(val);
			}

			value::value(float val) : value()
			{
				_first.set_float(val);
			}

			value::value(const char* str, bool allocated) : value()
			{
				_first.set_string(str, allocated);
			}

			value::value(uint32_t val) : value()
			{
				_first.set_uint(val);
			}

			value::value(const data& d) : value()
			{
				_first = d;
			}

			value::value(uint32_t val, internal::data_type t)
			{
				_first.set_uint(val);
				_first.type = t;
			}

			value_type value::type() const
			{
				// If we have multiple values set, then we are a string
				if (_second.type != data_type::none)
					return value_type::string;

				switch (_first.type)
				{
				case data_type::int32:
					return value_type::integer;
				case data_type::uint32:
					return value_type::divert;
				case data_type::float32:
					return value_type::decimal;
				case data_type::string_table_pointer:
				case data_type::allocated_string_pointer:
					return value_type::string;
				case data_type::null:
					return value_type::null;
				default:
					inkFail("Invalid data in value container!");
				}
			}

			void value::cast(value& val, value_type old_type, value_type new_type)
			{
				if (old_type == new_type)
					return;

				inkAssert(old_type < new_type, "Can only cast values upwards!");
				inkAssert(new_type != value_type::null && old_type != value_type::null, "Can not cast void values");
				inkAssert(new_type != value_type::divert && old_type != value_type::divert, "Can not cast divert values");

				// We do not actually convert floats/ints to strings here.
				//  Instead, we just pass along the float and it is appended to 
				//  another value's data stream
				if (new_type == value_type::string)
					return;

				switch (old_type)
				{
				case value_type::integer:
					if (new_type == value_type::decimal)
						val = value((float)val._first.integer_value);
					return;
				case value_type::decimal:
					if (new_type == value_type::integer)
						val = value((int)val._first.float_value);
					return;
				default:
					break;
				}

				inkAssert(false, "Invalid value cast");
			}

			value_type value::maybe_cast(value& left, value& right)
			{
				// Check the types of the two values. If they're the same, nothing to do.
				auto l_type = left.type(), r_type = right.type();
				if (l_type == r_type)
					return l_type;

				// Find the "largest" type
				value_type type = l_type > r_type ? l_type : r_type;

				// Cast
				value::cast(left, l_type, type);
				value::cast(right, r_type, type);

				// Return new type
				return type;
			}

			void value::mark_strings(string_table& strings) const
			{
				// mark any allocated strings we're using
				for (int i = 0; i < VALUE_DATA_LENGTH; i++)
				{
					switch (_data[i].type)
					{
					case data_type::allocated_string_pointer:
						strings.mark_used(_data[i].string_val);
						break;
					case data_type::none:
						return;
					default:
						break;
					}
				}
			}

			bool value::is_truthy() const
			{
				// Concatenated strings are true
				if (_second.type != data_type::none)
					return true;

				switch (_first.type)
				{
				case data_type::int32:
					return _first.integer_value != 0;
				case data_type::float32:
					return _first.float_value != 0.0f;
				case data_type::string_table_pointer:
				case data_type::allocated_string_pointer:
					return _first.string_val[0] != '\0';
				default:
					break;
				}

				inkFail("Invalid type to check for truthy");
			}

			void value::append_to(basic_stream& out) const
			{
				size_t i = 0;
				for (; i < VALUE_DATA_LENGTH; i++)
				{
					if (_data[i].type == data_type::none)
						break;
				}
				out.append(&_first, i);;
			}

			void value::load_from(basic_stream& in)
			{
				in.get(&_first, VALUE_DATA_LENGTH);
			}

			value value::add(value left, value right, basic_stream& stream, string_table& table)
			{
				// Cast as needed
				value_type new_type = maybe_cast(left, right);

				switch (new_type)
				{
				case value_type::integer:
					return left.as_int() + right.as_int();
				case value_type::decimal:
					return left.as_float() + right.as_float();
				case value_type::string:
				{
					// Create empty value
					value new_value = value();

					// Copy left values into new
					int i = 0, j = 0;
					bool overflow = false;
					while (j < VALUE_DATA_LENGTH && left._data[j].type != data_type::none && !overflow)
					{
						if (i >= VALUE_DATA_LENGTH)
						{
							overflow = true;
							break;
						}
						new_value._data[i++] = left._data[j++];
					}

					// Copy right values into new
					j = 0;
					while (j < VALUE_DATA_LENGTH && right._data[j].type != data_type::none && !overflow)
					{
						if (i >= VALUE_DATA_LENGTH)
						{
							overflow = true;
							break;
						}
						new_value._data[i++] = right._data[j++];
					}

					// Todo: Use string buffer for dynamic allocation!
					if (overflow)
					{
						// Add a marker
						stream << marker;

						// Push everything into the stream
						j = 0;
						while (j < VALUE_DATA_LENGTH && left._data[j].type != data_type::none)
							stream << left._data[j++];
						j = 0;
						while (j < VALUE_DATA_LENGTH && right._data[j].type != data_type::none)
							stream << right._data[j++];

						// Pull out into a new string
						return value(stream.get_alloc(table), true);
					}

					return new_value;
				}
				default:
					break;
				}

				inkFail("Invalid type for add");
			}

			// TODO: Macro to make defining these easier when there's no string involvement?

			value value::subtract(value left, value right)
			{
				// Cast as needed
				value_type new_type = maybe_cast(left, right);

				switch (new_type)
				{
				case value_type::integer:
					return left.as_int() - right.as_int();
				case value_type::decimal:
					return left.as_float() - right.as_float();
				default:
					break;
				}

				inkFail("Invalid type for subtract");
			}

			value value::multiply(value left, value right)
			{
				// Cast as needed
				value_type new_type = maybe_cast(left, right);

				switch (new_type)
				{
				case value_type::integer:
					return left.as_int() * right.as_int();
				case value_type::decimal:
					return left.as_float() * right.as_float();
				default:
					break;
				}

				inkFail("Invalid type for multiply");
			}

			value value::divide(value left, value right)
			{
				// Cast as needed
				value_type new_type = maybe_cast(left, right);

				switch (new_type)
				{
				case value_type::integer:
					return left.as_int() / right.as_int();
				case value_type::decimal:
					return left.as_float() / right.as_float();
				default:
					break;
				}

				inkFail("Invalid type for divide");
			}

			value value::mod(value left, value right)
			{
				// Cast as needed
				value_type new_type = maybe_cast(left, right);

				switch (new_type)
				{
				case value_type::integer:
					return left.as_int() % right.as_int();
				default:
					break;
				}

				inkFail("Invalid type for mod");
			}

			void convert_to_string(const char*& c, const data& d, char* number) {
				c = number;
				switch(d.type) {
				case data_type::int32:
					snprintf(number, 32, "%d", d.integer_value);
					break;
				case data_type::uint32:
					snprintf(number, 32, "%d", d.uint_value);
					break;
				case data_type::float32:
					snprintf(number, 32, "%f", d.float_value);
					break;
				case data_type::newline:
					number[0] = '\n';
					number[1] = 0;
					break;
				case data_type::string_table_pointer:
				case data_type::allocated_string_pointer:
					c = d.string_val;
					break;
				default: number[0] = 0;
				}
			}

			void value::finalize_string(string_table& table) const {
				constexpr size_t MaxSize = 256; // max size for no string element
				char buffer[VALUE_DATA_LENGTH][MaxSize];
				const char* strs[VALUE_DATA_LENGTH];
				char null = 0;

				size_t len = 0;
				for (int i = 0; i < VALUE_DATA_LENGTH; ++i) {
					switch(_data[i].type) {
					case data_type::float32:
						strs[i] = buffer[i];
						toStr(buffer[i], MaxSize, _data[i].float_value);
						break;
					case data_type::uint32:
						strs[i] = buffer[i];
						toStr(buffer[i], MaxSize, _data[i].uint_value);
						break;
					case data_type::int32:
						strs[i] = buffer[i];
						toStr(buffer[i], MaxSize, _data[i].integer_value);
						break;
					case data_type::string_table_pointer:
					case data_type::allocated_string_pointer:
						strs[i] = _data[i].string_val;
						break;
					default: strs[i] = &null;
					}
					_data[i].set_none();
					len += strlen(strs[i]);
				}
				char* str = table.create(len+1);
				table.mark_used(str);

				char* ptr = str;
				for (int i = 0; i < VALUE_DATA_LENGTH; ++i) {
					for(const char* c = strs[i]; *c; ++c){
						*ptr++ = *c;
					}
				}
				*ptr = 0;
				_data[0].set_string(str, true);
			}

			const char* value::as_str(string_table& table) const {
				finalize_string(table);
				return _first.string_val;
			}

			const char * const * value::as_str_ptr(string_table& table) const {
				finalize_string(table);
				return &_first.string_val;
			}

			bool value::compare_string(const value& left, const value& right) {
				// convert fields to string representation and start comparison
				// when the end of one field is reached, the other still has
				// bytes left -> convert the next field and continue comparison

				// iterator for data fields of left and right value
				size_t l_i = 0;
				size_t r_i = 0;
				// buffer to store string representation of numeric fields
				char l_number[32]; l_number[0] = 0;
				char r_number[32]; r_number[0] = 0;
				// current compare position, if *l = 0 -> field compare finish
				const char* l_c = l_number;
				const char* r_c = r_number;
				bool res = true;
				// while no different found and fields to check remain
				while(res && l_i < VALUE_DATA_LENGTH && r_i < VALUE_DATA_LENGTH) {
					// if one field has left overs
					if (*l_c || *r_c)
					{
						// fetch the next field of the value without leftover
						if (*l_c) {
							convert_to_string(r_c, right._data[r_i], r_number);
						} else {
							convert_to_string(l_c, left._data[l_i], l_number);
						}
					}
					// if both values are aligned: check if both have the same type
					else if (left._data[l_i].type == right._data[r_i].type)
					{
						bool tmp_res = true;
						switch(left._data[l_i].type) {
						case data_type::int32:
							tmp_res = left._data[l_i].integer_value == right._data[r_i].integer_value;
							break;
						case data_type::uint32:
							tmp_res = left._data[l_i].uint_value == right._data[r_i].uint_value;
							break;
						case data_type::float32:
							tmp_res = left._data[l_i].float_value == right._data[r_i].float_value;
							break;
						case data_type::string_table_pointer:
						case data_type::allocated_string_pointer:
							l_c = left._data[l_i].string_val;
							r_c = right._data[r_i].string_val;
							break;
						default: break;
						}
						// check if maybe the missing part is in next data
						if (!tmp_res) {
							convert_to_string(r_c, right._data[r_i], r_number);
							convert_to_string(l_c, left._data[l_i], l_number);
						}
					}
					// convert both to string and compare
					else
					{
						convert_to_string(r_c, right._data[r_i], r_number);
						convert_to_string(l_c, left._data[l_i], l_number);
					}
					// compare string representation until one reaches the end
					while(*l_c && *r_c) {
						// if different found: stop and set result to false
						if (*l_c != *r_c) {
							res = false; break;
						}
						++l_c; ++r_c;
					}
					// if field is finished advance to the next
					if (!*l_c){ ++l_i; }
					if (!*r_c){ ++r_i; }
				}
				// if one value not complete compared -> leftover witch not match
				if (res && l_i != r_i) {
					const value& v = l_i < r_i ? left : right;
					int i = l_i < r_i ? l_i : r_i;
					// check if leftover fields all empty
					while(v._data[i].type != data_type::none) {
						if (v._data[i].type != data_type::string_table_pointer
								&& v._data[i].type != data_type::allocated_string_pointer) {
							if (*v._data[i].string_val != 0) {
								return false;
							}
						} else {
							return false;
						}
						++i;
					}
				}
				return res;
			}

			value value::is_equal(value left, value right)
			{
				// Cast as needed
				value_type new_type = maybe_cast(left, right);

				switch (new_type)
				{
				case value_type::integer:
					return left.as_int() == right.as_int();
				case value_type::decimal:
					return left.as_float() == right.as_float();
				case value_type::string:
					return compare_string(left, right);
				case value_type::divert:
					return left.as_divert() == right.as_divert();
				default:
					break;
				}

				inkFail("Invalid type for is_equal");
			}

			value value::less_than(value left, value right)
			{
				// Cast as needed
				value_type new_type = maybe_cast(left, right);

				switch (new_type)
				{
				case value_type::integer:
					return left.as_int() < right.as_int();
				case value_type::decimal:
					return left.as_float() < right.as_float();
				default:
					break;
				}

				inkFail("Invalid type for less_than");
			}

			value value::negate(const value& val)
			{
				inkAssert(val._second.type == data_type::none, "Can not negate strings");

				switch (val._first.type)
				{
				case data_type::int32:
					return -val._first.integer_value;
				case data_type::float32:
					return -val._first.float_value;
				default:
					break;
				}

				inkFail("Invalid type for negate");
			}

			basic_stream& operator>>(basic_stream& in, value& out)
			{
				out.load_from(in);
				return in;
			}

			basic_stream& operator<<(basic_stream& out, const value& in)
			{
				in.append_to(out);
				return out;
			}
		}
	}
}
