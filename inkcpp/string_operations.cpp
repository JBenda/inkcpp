#include "stack.h"
#include "value.h"
#include "string_utils.h"
#include "operations.h"
#include "string_table.h"

namespace ink::runtime::internal {
	namespace casting {
		/**
		 * @brief Wrapper to cast values to string.
		 * string representation is stored inside string_cast.
		 */
		class string_cast {
		public:
			string_cast(const value& val);
			const char* get() const { return _str; }
		private:
			const value& _val;
			const char* _str;
			char _data[512]; //TODO define central
		};

		/**
		 * @brief template for string conversion.
		 * @param buffer to store the string
		 * @param n length of buffer
		 * @param v value to convert
		 * @tparam ty value type of v
		 */
		template<value_type ty>
		int cast_to_string(char* buffer, size_t n, const value& v){
			ink_exception("cast not implemented");
			return -1;
		}

		/**
		 * @brief wrapper to call correct cast_to_string function.
		 */
		template<size_t N, value_type ty = value_type::BEGIN>
		int to_string(char data[N], const value& v) {
			if (v.type() == ty) {
				return cast_to_string<ty>(data, N, v);
			} else if constexpr (ty < value_type::OP_END) {
				return to_string<N,ty+1>(data, v);
			} else {
				throw ink_exception("cast target not exist!");
			}
		}

		// cast for int32
		template<>
		int cast_to_string<value_type::int32>(char* data, size_t size, const value& v) {
			return toStr(data, size, v.get<value_type::int32>());
		}

		// cast for uint32
		template<>
		int cast_to_string<value_type::uint32>(char* data, size_t size, const value& v) {
			return toStr(data, size, v.get<value_type::uint32>());
		}

		// cast for float32
		template<>
		int cast_to_string<value_type::float32>(char* data, size_t size, const value& v) {
			return toStr(data, size, v.get<value_type::float32>());
		}

		// cast for newline
		template<>
		int cast_to_string<value_type::newline>(char* data, size_t size, const value& v) {
			return toStr(data, size, "\n");
		}


		// constructor for string_cast class
		string_cast::string_cast(const value& val) : _val{val}, _str{nullptr} {
			if (val.type() == value_type::string) {
				// reference string if value is already a string
				_str = val.get<value_type::string>();
			} else {
				// convert else
				_str = _data;
				to_string<512>(_data, val);
			}
		}
	}
	void operation<Command::ADD, value_type::string, void>::operator()(eval_stack& stack, value* vals) {
		// convert values to strings
		casting::string_cast lh(vals[0]);
		casting::string_cast rh (vals[1]);

		// create new string with needed size
		char* str = _string_table.create(c_str_len(lh.get()) + c_str_len(rh.get()) + 1);

		// copy to new string
		char* dst = str;
		for(const char* src = lh.get(); *src; ++src) { *dst++ = *src; }
		for(const char* src = rh.get(); *src; ++src) { *dst++ = *src; }
		*dst = 0;

		stack.push(value{}.set<value_type::string>(str));
	}

	void operation<Command::IS_EQUAL, value_type::string, void>::operator()(eval_stack& stack, value* vals) {
		// convert values to string
		casting::string_cast lh (vals[0]);
		casting::string_cast rh(vals[1]);

		// compare strings char wise
		const char* li = lh.get();
		const char* ri = rh.get();
		while(*li && *ri && *li++ == *ri++);

		stack.push(value{}.set<value_type::boolean>(*li == *ri));
	}

}
