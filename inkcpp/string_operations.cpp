/// implementation for commands on strings
/// string_cast is a class which convert an value to a string.
/// if the value is already a string it dose nothing (just serve the pointer),
/// else it convert the value to a string and store it, in it internal storage.

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

		// constructor for string_cast class
		string_cast::string_cast(const value& val) : _val{val}, _str{nullptr} {
			if (val.type() == value_type::string) {
				// reference string if value is already a string
				_str = val.get<value_type::string>();
			} else {
				// convert else
				_str = _data;
				toStr(_data, 512, val);
			}
		}
	}
	void operation<Command::ADD, value_type::string, void>::operator()(basic_eval_stack& stack, value* vals) {
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

	void operation<Command::IS_EQUAL, value_type::string, void>::operator()(basic_eval_stack& stack, value* vals) {
		// convert values to string
		casting::string_cast lh (vals[0]);
		casting::string_cast rh(vals[1]);

		// compare strings char wise
		const char* li = lh.get();
		const char* ri = rh.get();
		while(*li && *ri && *li == *ri) { ++li; ++ri; }

		stack.push(value{}.set<value_type::boolean>(*li == *ri));
	}

	void operation<Command::NOT_EQUAL, value_type::string, void>::operator()(basic_eval_stack& stack, value* vals) {
		// convert values to string
		casting::string_cast lh (vals[0]);
		casting::string_cast rh(vals[1]);

		// compare strings char wise
		const char* li = lh.get();
		const char* ri = rh.get();
		while(*li && *ri && *li == *ri){ ++li; ++ri; }

		stack.push(value{}.set<value_type::boolean>(*li != *ri));
	}

	bool has(const char* lh, const char* rh) {
		while(isspace(*lh)) { ++lh; }
		while(isspace(*rh)) { ++rh; }
		if(!*lh && !*rh) { return true; }
		for(const char* li = lh; *li; ++li) {
			const char* ri = rh;
			bool match = true;
			int offset = 0;
			for(int i = 0; ri[i] != 0; ++i) {
				if(li[i + offset] != ri[i]) {
					if(isspace(ri[i])) {
						--offset;
						continue;
					}
					match = false; break;
				}
			}
			if(match) { return true; }
		}
		return false;
	}

	void operation<Command::HAS, value_type::string, void>::operator()(basic_eval_stack& stack, value* vals)
	{
		casting::string_cast lh(vals[0]);
		casting::string_cast rh(vals[1]);
		stack.push(value{}.set<value_type::boolean>(has(lh.get(), rh.get())));
	}

	void operation<Command::HASNT, value_type::string, void>::operator()(basic_eval_stack& stack, value* vals)
	{
		casting::string_cast lh(vals[0]);
		casting::string_cast rh(vals[1]);
		stack.push(value{}.set<value_type::boolean>(!has(lh.get(), rh.get())));
	}

}
