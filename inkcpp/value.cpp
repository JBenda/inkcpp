#include "value.h"
#include "output.h"

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
					assert(false, "Invalid data in value container!");
				}
			}

			void value::cast(value& val, value_type old_type, value_type new_type)
			{
				if (old_type == new_type)
					return;

				assert(old_type < new_type, "Can only cast values upwards!");
				assert(new_type != value_type::null && old_type != value_type::null, "Can not cast void values");
				assert(new_type != value_type::divert && old_type != value_type::divert, "Can not cast divert values");

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
				}

				assert(false, "Invalid value cast");
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
				}

				assert(false, "Invalid type to check for truthy");
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

			value value::add(value left, value right)
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
					while (left._data[j].type != data_type::none && j < VALUE_DATA_LENGTH && i < VALUE_DATA_LENGTH)
						new_value._data[i++] = left._data[j++];

					// Copy right values into new
					j = 0;
					while (right._data[j].type != data_type::none && j < VALUE_DATA_LENGTH && i < VALUE_DATA_LENGTH)
						new_value._data[i++] = right._data[j++];

					// TODO: Test for overflow?
					return new_value;
				}
				}

				assert(false, "Invalid type for add");
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
				}

				assert(false, "Invalid type for subtract");
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
				}

				assert(false, "Invalid type for multiply");
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
				}

				assert(false, "Invalid type for divide");
			}

			value value::mod(value left, value right)
			{
				// Cast as needed
				value_type new_type = maybe_cast(left, right);

				switch (new_type)
				{
				case value_type::integer:
					return left.as_int() % right.as_int();
				}

				assert(false, "Invalid type for mod");
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
					break; // TODO: data[] operators?
				}

				assert(false, "Invalid type for is_equal");
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
				}

				assert(false, "Invalid type for less_than");
			}

			value value::negate(const value& val)
			{
				assert(val._second.type == data_type::none, "Can not negate strings");

				switch (val._first.type)
				{
				case data_type::int32:
					return -val._first.integer_value;
				case data_type::float32:
					return -val._first.float_value;
				}

				assert(false, "Invalid type for negate");
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