#pragma once

/// Managing casting between value types.
/// The casting is defined by an NxN matrix where N = |value_types|.
/// The entry m,n is the type where we cast to when rh = value_type(m) and
/// lh = value_type(n).
/// for that the matrix is symmetric.
/// `value_type::none` is used to mark an invalid cast
///
/// The entries are set in the `set_cast` function, which iterates over all
/// value_types combination. For each combination it checks the value of
/// `cast<v1,v2>` (with v1 < v2). When not other defined it is none.

#include "value.h"

namespace ink::runtime::internal::casting {

	/**
	 * @brief casting_matrix data and access wrapper.
	 */
	struct casting_matrix_type {
	public:
		constexpr casting_matrix_type() : _data{value_type::none}{};
		constexpr value_type get(value_type t1, value_type t2) const {
			return _data[static_cast<size_t>(t1)*N+static_cast<size_t>(t2)];
		}
		static constexpr size_t N = static_cast<size_t>(value_type::OP_END);
		value_type _data[N*N];
	};

	// iterate through each value_type combination and populate the
	// casting_matrix
	template<value_type t1 = value_type::BEGIN, value_type t2 = value_type::BEGIN>
	constexpr void set_cast (value_type data[casting_matrix_type::N*casting_matrix_type::N]){

		if constexpr (t2 == value_type::OP_END) {
			// end reached
		} else if constexpr (t1 == value_type::OP_END) {
			// go to next row
			set_cast<value_type::BEGIN, t2+1>(data);
		} else {
			// get entry from cast<t1,t2>
			constexpr size_t n1 = static_cast<size_t>(t1);
			constexpr size_t n2 = static_cast<size_t>(t2);
			// set matrix entry
			if constexpr (n1 < n2) {
				data[n1*casting_matrix_type::N + n2] = cast<t1,t2>::value;
			} else {
				data[n1*casting_matrix_type::N + n2] = cast<t2,t1>::value;
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
