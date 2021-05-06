#include "value.h"
#include "output.h"

namespace ink::runtime::internal
{
#ifdef INK_ENABLE_STL
	template<value_type ty = value_type::PRINT_BEGIN>
	void append(std::ostream& os, const value& val) {
		if constexpr (ty != value_type::PRINT_END) {
			if (ty == val.type()) {
				os << val.get<ty>();
			} else {
				append<ty+1>(os, val);
			}
		}
	}

	template<>
	void append<value_type::list_flag>(std::ostream& os, const value& val) {
		if (val.type() == value_type::list_flag) {
			os << "No list flag serilasation implementet yet!"; // TODO: implement
		} else {
			append<value_type::list_flag + 1>(os, val);
		}
	}
	template<>
	void append<value_type::list>(std::ostream& os, const value& val) {
		if (val.type() == value_type::list) {
			os << "No list serilasation implementet yet!"; // TODO: implement
		} else {
			append<value_type::list +1>(os, val);
		}
	}

	std::ostream& operator<<(std::ostream& os, const value& val) {
		if (val.type() < value_type::PRINT_BEGIN || val.type() >= value_type::PRINT_END) {
			throw ink_exception("printing this type is not supported");
		}
		append(os, val);
		return os;
	}
#endif

	basic_stream& operator<<(basic_stream& os, const value& val) {
		os.append(val);
		return os;
	}

	basic_stream& operator>>(basic_stream& is, value& val) {
		// FIXME: bullshit!
		is.get(&val, 1);
		return is;
	}
}
