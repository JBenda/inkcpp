#pragma once

#include "system.h"

namespace ink::runtime
{
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
		 * @return nullptr if variable with this name don't exist or
		 *                 the type differs.
		 * @return pointer to variable
		 */
		template<typename T>
		T* get(const char* name) {
			static_assert(
					internal::always_false<T>::value,
					"Requested Type is not supported");
		}
		template<typename T>
		const T* get(const char* name) const {
			static_assert(
					internal::always_false<T>::value,
					"Requested Type is not supported");
		}

		virtual ~globals_interface() = default;

	protected:
		virtual const uint32_t* getUInt(hash_t name) const = 0;
		virtual uint32_t* getUInt(hash_t name) = 0;
		virtual const int32_t* getInt(hash_t name) const = 0;
		virtual int32_t* getInt(hash_t name) = 0;
		virtual const float* getFloat(hash_t name) const = 0;
		virtual float* getFloat(hash_t name) = 0;
		virtual const char* getStr(hash_t name) const = 0;
		virtual char* getStr(hash_t name) = 0;
	};

	template<>
	inline const uint32_t* globals_interface::get<uint32_t>(const char* name) const {
		return getUInt(hash_string(name));
	}
	template<>
	inline uint32_t* globals_interface::get<uint32_t>(const char* name) {
		return getUInt(hash_string(name));
	}

	template<>
	inline const int32_t* globals_interface::get<int32_t>(const char* name) const {
		return getInt(hash_string(name));
	}
	template<>
	inline int32_t* globals_interface::get<int32_t>(const char* name) {
		return getInt(hash_string(name));
	}

	template<>
	inline const float* globals_interface::get<float>(const char* name) const {
		return getFloat(hash_string(name));
	}
	template<>
	inline float* globals_interface::get<float>(const char* name) {
		return getFloat(hash_string(name));
	}

	template<>
	inline const char* globals_interface::get<char>(const char* name) const {
		return getStr(hash_string(name));
	}
	template<>
	inline char* globals_interface::get<char>(const char* name) {
		return getStr(hash_string(name));
	}
}
