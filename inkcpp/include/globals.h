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
			return get_impl<T>(hash_string(name));
		}
		template<typename T>
		const T* get(const char* name) const {
			return get_impl<T>(hash_string(name));
		}

		virtual ~globals_interface() = default;

	protected:
		template<typename T>
		T* get_impl(hash_t name) const {
			static_assert(
					internal::always_false<T>::value,
					"Requested Type is not supported");
		}
		virtual uint32_t* getUInt(hash_t name) const = 0;
		virtual int32_t* getInt(hash_t name) const = 0;
		virtual float* getFloat(hash_t name) const = 0;
		virtual char* getStr(hash_t name) const = 0;
	};

	template<>
	inline uint32_t* globals_interface::get_impl<uint32_t>(hash_t name) const {
		return getUInt(name);
	}

	template<>
	inline int32_t* globals_interface::get_impl<int32_t>(hash_t name) const {
		return getInt(name);
	}

	template<>
	inline float* globals_interface::get_impl<float>(hash_t name) const {
		return getFloat(name);
	}

	template<>
	inline char* globals_interface::get_impl<char>(hash_t name) const {
		return getStr(name);
	}
}
