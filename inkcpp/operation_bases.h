#pragma once

namespace ink::runtime::internal {
	class string_table;

	template<typename ...>
	class operation_base {
	public:
		template<typename T>
		operation_base(const T&) { static_assert(always_false<T>::value, "use undefined base!"); }
	};

	template<>
	class operation_base<void> {
	public:
		template<typename T>
		operation_base(const T&) {}
	};

	template<>
	class operation_base<string_table> {
	public:
		template<typename T>
		operation_base(const T& t) : _string_table{*std::get<string_table*>(t)} {}

	private:
		string_table& _string_table;
	};
}
