#pragma once

#include "system.h"

#ifdef INK_ENABLE_STL
#include <string>
#endif

namespace ink
{
	namespace runtime 
	{
		namespace internal 
		{
			class string_table;

			// Data types that can be held internally within the ink runtime
			enum class data_type
			{
				none,							// blank. used when data is in a fixed array
				int32,							// 32-bit integer value
				float32,						// 32-bit floating point value
				uint32,							// 32-bit unsigned integer value
				string_table_pointer,			// Represents an offset within the story's constant string table
				allocated_string_pointer,		// Represents an offset within the runner's allocated string table
				marker,							// Special marker (used in output stream)
				glue,							// Glue.
				newline,						// \n
				func_start,						// Start of function marker
				func_end,						// End of function marker
				null,							// void/null (used for void function returns)
				tunnel_frame,					// Return from tunnel
				function_frame,					// Return from function
				thread_frame,					// Special tunnel marker for returning from threads
				thread_start,					// Start of a new thread frame
				thread_end,						// End of a thread frame
				jump_marker,					// Used to mark a callstack jump
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

			// TODO: Re-implement. Failed on 64-bit builds.
			//static_assert(sizeof(data) == sizeof(data_type) + sizeof(offset_t), "No data type should take up more than 32 bits");

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
				value(uint32_t, data_type);			// Create divert with type

				// Create a new string value (must specify whether or not it's an allocated or story string)
				value(const char*, bool allocated = false);

				// Check the value's current type
				value_type type() const;
				bool is_none() const { return _first.type == data_type::none; }
				data_type get_data_type() const { return _first.type; }

				// == Getters ==
				int as_int() const { return _first.integer_value; }
				float as_float() const { return _first.float_value; }
				uint32_t as_divert() const { return _first.uint_value; }
				uint32_t as_thread_id() const { return _first.uint_value; }
				// TODO: String access?

				template<typename T>
				T get() const { static_assert(always_false<T>::value, "Type not supported by value class"); }

				// Garbage collection
				void mark_strings(string_table&) const;

				inline operator int() const { return as_int(); }
				inline operator float() const { return as_float(); }
				inline operator uint32_t() const { return as_divert(); }

				// == Threading ==
				inline bool is_thread_marker() const { return _first.type == data_type::thread_start || _first.type == data_type::thread_end; }
				inline bool is_thread_end() const { return _first.type == data_type::thread_end; }
				inline bool is_thread_start() const { return _first.type == data_type::thread_start; }
				inline bool is_jump_marker() const { return _first.type == data_type::jump_marker; }
				inline uint32_t& thread_jump() { return _second.uint_value; }
				inline uint32_t thread_jump() const { return _second.uint_value; }

				// Is this value "true"
				bool is_truthy() const;
				inline operator bool() const { return is_truthy(); }

				void append_to(basic_stream&) const;
				void load_from(basic_stream&);

				// == Binary operations ==
				static value add(value, value, basic_stream&, string_table&);
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
			// inline value operator+(const value& lhs, const value& rhs) { return value::add(lhs, rhs); }
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

			// == Specialized get functions ==
			// TODO: Should these assert?

			template<>
			inline int value::get<int>() const { return as_int(); }
			template<>
			inline float value::get<float>() const { return as_float(); }
			template<>
			inline uint32_t value::get<uint32_t>() const { return as_divert(); }

#ifdef INK_ENABLE_STL
			template<>
			inline std::string value::get<std::string>() const { return _first.string_val; } // TODO: Missing amalgamate?
#endif
#ifdef INK_ENABLE_UNREAL
			template<>
			inline FString value::get<FString>() const { return _first.string_val; } // TODO: Missing amalgamate?
#endif
		}
	}
}