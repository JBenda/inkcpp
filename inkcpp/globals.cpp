#include "globals.h"
#include "globals_impl.h"
#include "value.h"

#include <compare>

namespace ink::runtime {
	template<>
	auto global_string<false>::operator<=>(const global_string& other) const {
		const char* m = _string;
		const char* o = other._string;
		while(*m && *o) {
			if (auto cmp = *m <=> *o; cmp != 0) { return cmp; }
			++m;
			++o;
		}
		if (*o) { return std::strong_ordering::less; }
		if (*m) { return std::strong_ordering::greater; }
		return std::strong_ordering::equal;
	}
	template<>
	global_string<false>::global_string(const internal::globals_impl& globals, const char* string)
		: _globals{globals}, _string{string}, _size{0}
	{
		if (*this) { for(const char* ptr = _string; *ptr; ++ptr) { ++_size; } }
	}

	global_string<true>::global_string(internal::globals_impl& globals, const char* string, hash_t name)
		: _globals{globals}, _name{name}, global_string<false>(globals, string)
	{}

	void global_string<true>::set(const char* string) {
		if (!*this) { ink_exception("try to set unbound variable!"); }
		size_t new_size = 0;
		char* ptr;
		for(const char* i = string; *i; ++i) { ++new_size; }
		internal::value* v = _globals.get_variable(_name);
		char* new_string = _globals.strings().create(new_size + 1);
		_globals.strings().mark_used(new_string);
		ptr = new_string;
		for(const char* i = string; *i; ++ptr, ++i) {
			*ptr = *i;
		}
		internal::data d;
		d.set_string(new_string, true);
		*v = internal::value(d);
		*ptr = 0;
		global_string<false>::_size = new_size;
		global_string<false>::_string = new_string;
	}
}
