#pragma once

#include "value.h"

namespace ink::runtime::internal {
	namespace casting {
		template<size_t N>
		value_type common_base(const value* vs) {
			if constexpr (N == 0) { return value_type::none; }
			else { return vs->type(); }
		}
	};
}
