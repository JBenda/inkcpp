#pragma once

#include <tuple>

namespace ink::runtime::internal {
	class string_table;

	template<typename ...>
	class operation_base {
	public:
		static constexpr bool enabled = false;
		template<typename T>
		operation_base(const T&) { static_assert(always_false<T>::value, "use undefined base!"); }
	};

	template<>
	class operation_base<void> {
	public:
		static constexpr bool enabled = true;
		template<typename T>
		operation_base(const T&) {}
	};

	template<>
	class operation_base<string_table> {
	public:
		static constexpr bool enabled = true;
		template<typename T>
		operation_base(const T& t) : _string_table{*std::get<string_table*>(t)} {}

	protected:
		string_table& _string_table;
	};
}
