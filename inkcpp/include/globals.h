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
		 */
		template<typename T>
		T* get(const char* name);

		virtual ~globals_interface() = default;

	protected:
		virtual uint32_t* getUInt(const char* name) = 0;
		virtual int32_t* getInt(const char* name) = 0;
		virtual float* getFloat(const char* name) = 0;
		virtual char* getStr(const char* name) = 0;
	};

	template<>
	uint32_t* globals_interface::get<uint32_t>(const char* name) {
		return getUInt(name);
	}

	template<>
	int32_t* globals_interface::get<int32_t>(const char* name) {
		return getInt(name);
	}

	template<>
	float* globals_interface::get<float>(const char* name) {
		return getFloat(name);
	}

	template<>
	char* globals_interface::get<char>(const char* name) {
		return getStr(name);
	}
}
