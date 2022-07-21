#pragma once

#include "system.h"

namespace ink::runtime
{
	class globals_interface;
	namespace internal { class globals_impl;}

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

		virtual ~globals_interface() = default;

	protected:
		virtual optional<value> get_var(hash_t name) const = 0;
		virtual bool set_var(hash_t name, const value& val) = 0;
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
			return {v->v_int32};
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
			return {v->v_float};
		}
		return nullopt;
	}
	template<>
	inline bool globals_interface::set<float>(const char* name, const float& val) {
		return set_var(string_hash(name), value(val));
	}
	
	template<>
	inline optional<const char*> globals_interface::get<const char*>(const char* name) const {
		auto var = get_var(hash_string(name));
		if ( var && var->type == value::Type::String ) {
			return {v->v_string};
		}
		return nullopt;
	}
	template<>
	inline bool globals_interface::set<const char*>(const char* name, const char*& val) {
		return set_var(string_hash(name), value(val));
	}
}
