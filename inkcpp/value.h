#pragma once

#include "system.h"

namespace ink
{
	namespace runtime 
	{
		namespace internal 
		{
			// Data types that can be held internally within the ink runtime
			enum class data_type
			{
				none,							// blank. used when data is in a fixed array
				int32,							// 32-bit integer value
				float32,						// 32-bit floating point value
				uint32,							// 32-bit unsigned integer value
				string_table_pointer,			// Represents an offset within the story's constant string table
				allocated_string_pointer,		// Represents an offset within the runner's allocated string table (TODO)
				marker,							// Special marker (used in output stream)
				glue,							// Glue.
				newline,						// \n
				null,							// void/null (used for void function returns)
			};

			// Container for any data used as part of the runtime (variable values, output streams, evaluation stack, etc.)
			struct data
			{
				// Type of data
				data_type type;

				union
				{
					int integer_value;
					uint32_t uint_value;
					float float_value;
					const char* string_val;
					// TODO: Do we need a marker type?
				};

				inline void set_none() { type = data_type::none; }
				inline void set_void() { type = data_type::null; }
				inline void set_int(int val) { type = data_type::int32; integer_value = val; }
				inline void set_uint(uint32_t val) { type = data_type::uint32; uint_value = val; }
				inline void set_float(float val) { type = data_type::float32; float_value = val; }
				inline void set_string(const char* val, bool allocated) { type = allocated ? data_type::allocated_string_pointer : data_type::string_table_pointer; string_val = val; }
			};

			static_assert(sizeof(data) == sizeof(data_type) + sizeof(offset_t), "No data type should take up more than 32 bits");

			// Types of values
			enum class value_type
			{
				null,
				divert,
				integer,
				decimal,
				string,
			};

			class basic_stream;

			// Used to store values on the evaluation stack or variables.
			class value
			{
			public:
				value();							// Creates a value with the "none" type
				value(int);							// Create a new int value
				value(float);						// Create a new float value
				value(uint32_t);					// Create a new divert value
				value(const data&);					// Create value from data

				// Create a new string value (must specify whether or not it's an allocated or story string)
				value(const char*, bool allocated = false);

				// Check the value's current type
				value_type type() const;

				// == Getters ==
				int as_int() const { return _first.integer_value; }
				float as_float() const { return _first.float_value; }
				uint32_t as_divert() const { return _first.uint_value; }
				// TODO: String access?

				// Is this value "true"
				bool is_truthy() const;
				inline operator bool() const { return is_truthy(); }

				void append_to(basic_stream&) const;
				void load_from(basic_stream&);

				// == Binary operations ==
				static value add(value, value);
				static value subtract(value, value);
				static value multiply(value, value);
				static value divide(value, value);
				static value mod(value, value);
				static value is_equal(value, value);
				static value less_than(value, value);

				// == Unary operations
				static value negate(const value&);

			private:
				static void cast(value&, value_type, value_type);
				static value_type maybe_cast(value& left, value& right);

			private:
				// Maximum sequential data a value can have
				static const size_t VALUE_DATA_LENGTH = 4;

				union
				{
					// Quick access struct
					struct 
					{
						data _first; 
						data _second;
					};
					
					// Data array
					data _data[VALUE_DATA_LENGTH];
				};
				
			};

			// == Binary Operators ==
			inline value operator+(const value& lhs, const value& rhs) { return value::add(lhs, rhs); }
			inline value operator-(const value& lhs, const value& rhs) { return value::subtract(lhs, rhs); }
			inline value operator*(const value& lhs, const value& rhs) { return value::multiply(lhs, rhs); } 
			inline value operator/(const value& lhs, const value& rhs) { return value::divide(lhs, rhs); }
			inline value operator%(const value& lhs, const value& rhs) { return value::mod(lhs, rhs); }
			inline value operator-(const value& val) { return value::negate(val); }

			inline value operator==(const value& lhs, const value& rhs) { return value::is_equal(lhs, rhs); }
			inline value operator!=(const value& lhs, const value& rhs) { return !value::is_equal(lhs, rhs); }
			inline value operator<(const value& lhs, const value& rhs) { return value::less_than(lhs, rhs); }
			inline value operator<=(const value& lhs, const value& rhs) { return value::less_than(lhs, rhs) || value::is_equal(lhs, rhs); }
			inline value operator>(const value& lhs, const value& rhs) { return !(lhs <= rhs); }
			inline value operator>=(const value& lhs, const value& rhs) { return !value::less_than(lhs,rhs); }

			// == Stream Operators ==
			basic_stream& operator <<(basic_stream&, const value&);
			basic_stream& operator >>(basic_stream&, value&);
		}
	}
}