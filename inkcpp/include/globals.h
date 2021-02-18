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
		virtual optional<uint32_t> get_uint(hash_t name) const = 0;
		virtual bool set_uint(hash_t name, uint32_t val) = 0;
		virtual optional<int32_t> get_int(hash_t name) const = 0;
		virtual bool set_int(hash_t name, int32_t val) = 0;
		virtual optional<float> get_float(hash_t name) const = 0;
		virtual bool set_float(hash_t name, float val) = 0;
		virtual optional<const char *>  get_str(hash_t name) const = 0;
		virtual bool set_str(hash_t name, const char* val) = 0;
	};

	template<>
	inline optional<uint32_t>  globals_interface::get<uint32_t>(const char* name) const {
		return get_uint(hash_string(name));
	}
	template<>
	inline bool globals_interface::set<uint32_t>(const char* name, const uint32_t& val) {
		return set_uint(hash_string(name), val);
	}

	template<>
	inline optional<int32_t> globals_interface::get<int32_t>(const char* name) const {
		return get_int(hash_string(name));
	}
	template<>
	inline bool globals_interface::set<int32_t>(const char* name, const int32_t& val) {
		return set_int(hash_string(name), val);
	}

	template<>
	inline optional<float> globals_interface::get<float>(const char* name) const {
		return get_float(hash_string(name));
	}
	template<>
	inline bool globals_interface::set<float>(const char* name, const float& val) {
		return set_float(hash_string(name), val);
	}

	template<>
	inline optional<const char*>globals_interface::get<const char*>(const char* name) const {
		return get_str(hash_string(name));
	}
	template<>
	inline bool globals_interface::set<const char*>(const char* name, const char * const & val) {
		return set_str(hash_string(name), val);
	}
}
