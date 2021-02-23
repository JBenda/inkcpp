#pragma once

#include "value.h"

namespace ink::runtime::internal::casting {

	class casting_matrix_type {
	public:
		constexpr value_type get(value_type t1, value_type t2) const {
			return _data[static_cast<size_t>(t1)][static_cast<size_t>(t2)];
		}
		static constexpr size_t N = static_cast<size_t>(value_type::OP_END);
		value_type _data[N][N];
	};

	template<value_type t1 = value_type::BEGIN, value_type t2 = value_type::BEGIN>
	constexpr void set_cast (value_type data[static_cast<size_t>(value_type::OP_END)][static_cast<size_t>(value_type::OP_END)]){
		if constexpr (t2 == value_type::OP_END) {
		} else if constexpr (t1 == value_type::OP_END) {
			set_cast<value_type::BEGIN, t2+1>(data);
		} else {
			constexpr size_t n1 = static_cast<size_t>(t1);
			constexpr size_t n2 = static_cast<size_t>(t2);
			if constexpr (n1 < n2) {
				data[n1][n2] = cast<t1,t2>;
			} else {
				data[n1][n2] = cast<t2,t1>;
			}
			set_cast<t1+1,t2>(data);
		}
	}

	constexpr casting_matrix_type construct_casting_matrix() {
		casting_matrix_type cm;
		set_cast(cm._data);
		return cm;
	}
	static constexpr casting_matrix_type casting_matrix = construct_casting_matrix();

	template<size_t N>
	value_type common_base(const value* vs) {
		if constexpr (N == 0) { return value_type::none; }
		else if constexpr (N == 1) { return vs->type(); }
		else {
			value_type ty = vs[0].type();
			for(size_t i = 1; i < N; ++i) {
				ty = casting_matrix.get(ty, vs[i].type());
			}
			return ty;
		}
	}
}
