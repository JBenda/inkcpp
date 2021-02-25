#pragma once

#include "value.h"

namespace ink::runtime::internal::casting {

	/**
	 * @brief casting_matrix data and access wrapper.
	 */
	struct casting_matrix_type {
	public:
		constexpr value_type get(value_type t1, value_type t2) const {
			return _data[static_cast<size_t>(t1)][static_cast<size_t>(t2)];
		}
		static constexpr size_t N = static_cast<size_t>(value_type::OP_END);
		value_type _data[N][N];
	};

	// iterate through each value_type combination and populate the
	// casting_matrix
	template<value_type t1 = value_type::BEGIN, value_type t2 = value_type::BEGIN>
	constexpr void set_cast (value_type data[static_cast<size_t>(value_type::OP_END)][static_cast<size_t>(value_type::OP_END)]){
		if constexpr (t2 == value_type::OP_END) {
			// end reached
		} else if constexpr (t1 == value_type::OP_END) {
			// go to next row
			set_cast<value_type::BEGIN, t2+1>(data);
		} else {
			// get entry from cast<t1,t2>
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

	// function to populate casting_matrix
	constexpr casting_matrix_type construct_casting_matrix() {
		casting_matrix_type cm;
		set_cast(cm._data);
		return cm;
	}

	/// NxN matrix which contains in cell i,j the common base of value_type(i)
	/// and value_type(j).
	static constexpr casting_matrix_type casting_matrix = construct_casting_matrix();

	/**
	 * @brief returns a type where each value can be casted to.
	 *        Result based on `cast<value_type,value_type> = value_type`
	 *        definitions.
	 * @tparam N number of values/value array length
	 * @param vs array which contains the values
	 * @return value_type::none if there is no common base defined
	 * @return common base of types in vs else
	 */
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
