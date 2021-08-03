#pragma once

/// defines data storage for operations.
/// provide constructor and handle the data member.
/// the data member therefore are protected accessible

#include "system.h"
#include "./tuple.hpp"
#include "random.h"

namespace ink::runtime {
	class runner_interface;
}
namespace ink::runtime::internal {
	class string_table;
	class list_table;
	class story_impl;
	class globals_impl;

	/// base class for operations to acquire data and provide flags and
	/// constructor
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

	// base class for operations which needs a string_table
	template<>
	class operation_base<string_table> {
	public:
		static constexpr bool enabled = true;
		template<typename T>
		operation_base(const T& t) : _string_table{*get<string_table*,T>(t)} {
			static_assert(has_type<string_table*,T>::value, "Executioner "
					"constructor needs a string table to instantiate "
					"some operations!");
		}

	protected:
		string_table& _string_table;
	};

	// base class for operations which needs a list_table
	template<>
	class operation_base<list_table> {
	public:
		static constexpr bool enabled = true;
		template<typename T>
		operation_base(const T& t) : _list_table{*get<list_table*,T>(t)} {
			static_assert(has_type<list_table*,T>::value, "Executioner "
					"constructor needs a list table to instantiate "
					"some operations!");
		}

	protected:
		list_table& _list_table;
	};

	// base class for operations which needs a list_table
	template<>
	class operation_base<prng> {
	public:
		static constexpr bool enabled = true;
		template<typename T>
		operation_base(const T& t) : _prng{*get<prng*,T>(t)} {
			static_assert(has_type<prng*,T>::value, "Executioner "
					"constructor needs a list table to instantiate "
					"some operations!");
		}

	protected:
		prng& _prng;
	};


	template<>
	class operation_base<list_table, prng> {
	public:
		static constexpr bool enabled = true;
		template<typename T>
		operation_base(const T& t)
		: _list_table{*get<list_table*,T>(t)}, _prng{*get<prng*,T>(t)}
		{
			static_assert(has_type<list_table*,T>::value, "Executioner "
					"constructor needs a list table to instantiate "
					"some operations!");
			static_assert(has_type<prng*,T>::value, "Executioner "
					"constructor needs a RNG to instantiate "
					"some operations!");
		}

	protected:
		list_table& _list_table;
		prng& _prng;
	};

	template<>
	class operation_base<const story_impl, globals_impl> {
		template<typename T>
		static const story_impl& get_story(const T& t) {
			if constexpr (has_type<const story_impl*,T>::value) {
				return *get<const story_impl*,T>(t);
			} else {
				static_assert(has_type<story_impl*,T>::value, "Executioner "
						"constructor needs a story_impl to instantiate "
						"container related operations!");
				return *get<story_impl*,T>(t);
			}
		}
	public:
		static constexpr bool enabled = true;
		template<typename T>
		operation_base(const T& t)
		: _story{get_story(t)}, _visit_counts{*get<globals_impl*,T>(t)}
		{
			static_assert(has_type<globals_impl*,T>::value, "Executioner "
					"constructor needs access to globals to instantiate "
					"container related operations!");
		}

	protected:
		const story_impl& _story;
		globals_impl& _visit_counts;
	};

	template<>
	class operation_base<const runner_interface> {
		template<typename T>
		static const runner_interface& get_const(const T& t) {
			if constexpr (has_type<const runner_interface*,T>::value) {
				return *get<const runner_interface*,T>(t);
			} else {
				static_assert(has_type<runner_interface*,T>::value,
						"Executioner constructor needs a runner to instantiate.");
				return *get<runner_interface*,T>(t);
			}
		}
	public:
		static constexpr bool enabled = true;
		template<typename T>
		operation_base(const T& t)
		: _runner{get_const(t)}
		{ }
	protected:
		const runner_interface& _runner;
	};
}
