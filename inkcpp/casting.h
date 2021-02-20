#pragma once

#include "value.h"

namespace ink::runtime::internal {
	namespace casting {
		template<size_t N>
		value_type common_base(const value* vs) {
			if constexpr (N == 0) { return value_type::none; }
			else if constexpr (N == 1) { return vs->type(); }
			else {
				if (vs[0].type() == value_type::string || vs[1].type() == value_type::string) {
					return value_type::string;
				} else {
					return vs[0].type();
				}
			}
		}
	};
}
