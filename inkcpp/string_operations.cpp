#include "stack.h"
#include "value.h"
#include "string_utils.h"
#include "operations.h"
#include "string_table.h"

namespace ink::runtime::internal {
	namespace casting {
		class string_cast {
		public:
			string_cast(const value& val);
			const char* get() const { return _str; }
		private:
			const value& _val;
			const char* _str;
			char _data[512]; //TODO define central
		};

		template<value_type ty>
		int cast_to_string(char* buffer, size_t n, const value& v){
			ink_exception("cast not implemented");
			return -1;
		}

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

		template<>
		int cast_to_string<value_type::int32>(char* data, size_t size, const value& v) {
			return toStr(data, size, v.get<value_type::int32>());
		}
		template<>
		int cast_to_string<value_type::uint32>(char* data, size_t size, const value& v) {
			return toStr(data, size, v.get<value_type::uint32>());
		}
		template<>
		int cast_to_string<value_type::float32>(char* data, size_t size, const value& v) {
			return toStr(data, size, v.get<value_type::float32>());
		}
		template<>
		int cast_to_string<value_type::newline>(char* data, size_t size, const value& v) {
			return toStr(data, size, "\n");
		}


		string_cast::string_cast(const value& val) : _val{val}, _str{nullptr} {
			if (val.type() == value_type::string) {
				_str = val.get<value_type::string>();
			} else {
				_str = _data;
				to_string<512>(_data, val);
			}
		}
	}
	void operation<Command::ADD, value_type::string, void>::operator()(eval_stack& stack, value* vals) {
		casting::string_cast lh(vals[0]);
		casting::string_cast rh (vals[1]);
		char* str = _string_table.create(strlen(lh.get()) + strlen(rh.get()) + 1);
		char* dst = str;
		for(const char* src = lh.get(); *src; ++src) { *dst++ = *src; }
		for(const char* src = rh.get(); *src; ++src) { *dst++ = *src; }
		*dst = 0;
		stack.push(value{}.set<value_type::string>(str));
	}

	void operation<Command::IS_EQUAL, value_type::string, void>::operator()(eval_stack& stack, value* vals) {
		casting::string_cast lh (vals[0]);
		casting::string_cast rh(vals[1]);
		const char* li = lh.get();
		const char* ri = rh.get();
		bool res;
		while(*li && *ri) {
			++li;
			++ri;
		}
		if (*li != 0 || *ri != 0) {
			res = false;
		} else {
			res = true;
		}
		stack.push(value{}.set<value_type::boolean>(res));
	}

}
