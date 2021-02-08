#pragma once

#include "system.h"

namespace ink::runtime
{
	class globals_interface;
	namespace internal { class globals_impl;}
	/**
	 * @brief wrapper for string in global store.
	 * @tparam WRITE if writing enable for this type
	 */
	template<bool WRITE = false>
	class global_string {
	public:
		operator bool() { return _string; }
		auto operator<=>(const global_string& other) const;
		char operator[](size_t i) const { return _string[i]; }; // TODO: boundary check?
		size_t size() const { return _size; };
		const char* data() const { return _string; }

	protected:
		friend class ink::runtime::internal::globals_impl;
		global_string(const internal::globals_impl& globals, const char* string);

		const internal::globals_impl& _globals;
		const char* _string;
		size_t _size;
	};
	template<>
	class global_string<true> : public global_string<false>{
	public:
		/**
		 * @brief define new value for string.
		 * The string will be copied in internal data structure there for.
		 */
		void set(const char* new_string);

		using global_string<false>::operator bool;
	protected:
		friend class ink::runtime::internal::globals_impl;
		global_string(internal::globals_impl& globals, const char* string, hash_t name);

		internal::globals_impl& _globals;
		hash_t _name;
	};
	template<typename T>
	struct return_type {
		using type = T*;
		using c_type = const T*;
	};
	template<typename T>
	using return_type_t = typename return_type<T>::type;
	template<typename T>
	using return_type_ct = typename return_type<T>::c_type;

	template<bool B>
	struct return_type<global_string<B>> {
		using type = global_string<true>;
		using c_type = global_string<false>;
	};

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
		return_type_t<T> get(const char* name) {
			static_assert(
					internal::always_false<T>::value,
					"Requested Type is not supported");
		}
		template<typename T>
		return_type_ct<T> get(const char* name) const {
			static_assert(
					internal::always_false<T>::value,
					"Requested Type is not supported");
		}

		virtual ~globals_interface() = default;

	protected:
		virtual const uint32_t* get_uint(hash_t name) const = 0;
		virtual uint32_t* get_uint(hash_t name) = 0;
		virtual const int32_t* get_int(hash_t name) const = 0;
		virtual int32_t* get_int(hash_t name) = 0;
		virtual const float* get_float(hash_t name) const = 0;
		virtual float* get_float(hash_t name) = 0;
		virtual global_string<false> get_str(hash_t name) const = 0;
		virtual global_string<true> get_str(hash_t name) = 0;
	};

	template<>
	inline const uint32_t*  globals_interface::get<uint32_t>(const char* name) const {
		return get_uint(hash_string(name));
	}
	template<>
	inline uint32_t* globals_interface::get<uint32_t>(const char* name) {
		return get_uint(hash_string(name));
	}

	template<>
	inline const int32_t* globals_interface::get<int32_t>(const char* name) const {
		return get_int(hash_string(name));
	}
	template<>
	inline int32_t* globals_interface::get<int32_t>(const char* name) {
		return get_int(hash_string(name));
	}

	template<>
	inline const float* globals_interface::get<float>(const char* name) const {
		return get_float(hash_string(name));
	}
	template<>
	inline float* globals_interface::get<float>(const char* name) {
		return get_float(hash_string(name));
	}

	template<>
	inline global_string<false> globals_interface::get<global_string<false>>(const char* name) const {
		return get_str(hash_string(name));
	}
	template<>
	inline global_string<true> globals_interface::get<global_string<true>>(const char* name) {
		return get_str(hash_string(name));
	}
}
