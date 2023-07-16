#pragma once

#include "types.h"
#include "functional.h"

namespace ink::runtime
{
	class snapshot;

	/**
	* Represents a global store to be shared amongst ink runners.
	* Stores global variable values, visit counts, turn counts, etc.
	*/
	class globals_interface
	{
	public:

		/**
		 * @brief Access global variable of Ink runner.
		 * @param name name of variable, as defined in InkScript
		 * @tparam T c++ type of variable
		 * @return nullopt if variable won't exist or type won't match
		 */
		template<typename T>
		optional<T> get(const char* name) const {
			static_assert(
					internal::always_false<T>::value,
					"Requested Type is not supported");
		}

		/**
		 * @brief Write new value in global store.
		 * @param name name of variable, as defined in InkScript
		 * @tparam T c++ type of variable
		 * @return true on success
		 */
		template<typename T>
		bool set(const char* name, const T& val) {
			static_assert(
					internal::always_false<T>::value,
					"Requested Type is not supported");
			return false;
		}

		/**
		 * @brief Observers global variable.
		 * 
		 * Calls callback with `value` or with casted value if it is one of 
		 * values variants. The callback will also be called with the current value
		 * when the observe is bind.
		 * @param callback functor with:
		 * * 0 arguments
		 * * 1 argument: `new_value`
		 * * 2 arguments: `new_value`, `ink::optional<old_value>`: first time call will not contain a old_value
		 */
		template<typename F>
		void observe(const char* name, F callback) {
			internal_observe(hash_string(name), new internal::callback(callback));
		}

		virtual snapshot* create_snapshot() const = 0;

		virtual ~globals_interface() = default;

	protected:
		virtual optional<value> get_var(hash_t name) const = 0;
		virtual bool set_var(hash_t name, const value& val) = 0;
		virtual void internal_observe(hash_t name, internal::callback_base* callback) = 0;
	};
	
	template<>
	inline optional<value> globals_interface::get<value>(const char* name) const {
		return get_var(hash_string(name));
	}
	template<>
	inline bool globals_interface::set<value>(const char* name, const value& val) {
		return set_var(hash_string(name), val);
	}
	
	template<>
	inline optional<bool> globals_interface::get<bool>(const char* name) const {
		auto var = get_var(hash_string(name));
		if ( var && var->type == value::Type::Bool) {
			return {var->v_bool};
		}
		return nullopt;
	}
	template<>
	inline bool globals_interface::set<bool>(const char* name, const bool& val) {
		return set_var(hash_string(name), value(val));
	}

	template<>
	inline optional<uint32_t>  globals_interface::get<uint32_t>(const char* name) const {
		auto var = get_var(hash_string(name));
		if (var && var->type == value::Type::Uint32) {
			return {var->v_uint32};
		}
		return nullopt;
	}
	template<>
	inline bool globals_interface::set<uint32_t>(const char* name, const uint32_t& val) {
		return set_var(hash_string(name), value(val));
	}

	template<>
	inline optional<int32_t> globals_interface::get<int32_t>(const char* name) const {
		auto var = get_var(hash_string(name));
		if (var && var->type == value::Type::Int32) {
			return {var->v_int32};
		}
		return nullopt;
	}
	template<>
	inline bool globals_interface::set<int32_t>(const char* name, const int32_t& val) {
		return set_var(hash_string(name), value(val));
	}
	
	
	template<>
	inline optional<float> globals_interface::get<float>(const char* name) const {
		auto var = get_var(hash_string(name));
		if ( var && var->type == value::Type::Float) {
			return {var->v_float};
		}
		return nullopt;
	}
	template<>
	inline bool globals_interface::set<float>(const char* name, const float& val) {
		return set_var(hash_string(name), value(val));
	}
	
	template<>
	inline optional<const char*> globals_interface::get<const char*>(const char* name) const {
		auto var = get_var(hash_string(name));
		if ( var && var->type == value::Type::String ) {
			return {var->v_string};
		}
		return nullopt;
	}
	template<>
	inline bool globals_interface::set<const char*>(const char* name, const char* const& val) {
		return set_var(hash_string(name), value(val));
	}

	template<>
	inline optional<list> globals_interface::get<list>(const char* name) const {
		auto var = get_var(hash_string(name));
		if (var && var->type == value::Type::List) {
			return {var->v_list};
		}
		return nullopt;
	}
	template<>
	inline bool globals_interface::set<list>(const char* name, const list& val) {
		return set_var(hash_string(name), value(val));
	}
}
