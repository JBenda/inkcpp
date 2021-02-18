#pragma once

#include "system.h"
#include "../shared/private/command.h"

#ifdef INK_ENABLE_STL
#include <iosfwd>
#endif


namespace ink::runtime::internal {
	class basic_stream;
	enum class value_type {
		BEGIN,
		none = BEGIN,
		divert,
		uint32,
		PRINT_BEGIN,
		boolean = PRINT_BEGIN,
		int32,
		float32,
		string,
		PRINT_END,
		marker = PRINT_END,
		glue,
		newline,
		func_start,
		func_end,
		null,
		tunnel_frame,
		function_frame,
		thread_start,
		thread_frame,
		thread_end,
		jump_marker,
		END};
	constexpr value_type operator+(value_type t, int i) {
		return static_cast<value_type>(static_cast<int>(t)+i);
	}
	constexpr Command operator+(Command c, int i) {
		return static_cast<Command>(static_cast<int>(c)+i);
	}


	class value {
	public:
		template<value_type> struct ret { using type = void; };

		constexpr value() : _type{value_type::none}, uint32_value{0}{}
		template<value_type ty>
		typename ret<ty>::type
		get() const { static_assert(ty != ty, "No getter for this type defined!"); }

		template<value_type ty, typename ...Args>
		constexpr value& set(Args ...args) {
			static_assert(sizeof...(Args)!=sizeof...(Args), "No setter for this type defined!");
			return *this;
		}

		value_type type() const { return _type; }
		friend basic_stream& operator<<(basic_stream& os, value);
		friend basic_stream& operator>>(basic_stream& is, value);

		constexpr bool printable() const { return _type >= value_type::PRINT_BEGIN && _type < value_type::PRINT_END; }
	private:
		union {
			bool bool_value;
			int32_t int32_value;
			const char* str_value;
			uint32_t uint32_value;
			float float_value;
		};
		value_type _type;
	};

#ifdef INK_ENABLE_STL
	std::ostream& operator<<(std::ostream&,const value&);
#endif
	template<> struct value::ret<value_type::int32> { using type = int32_t; };
	template<> inline int32_t value::get<value_type::int32>() const { return int32_value; }
	template<>
	inline constexpr value& value::set<value_type::int32, int32_t>(int32_t v) {
		int32_value = v;
		_type = value_type::int32;
		return *this;
	}

	template<> struct value::ret<value_type::divert> { using type = uint32_t; };
	template<> inline uint32_t value::get<value_type::divert>() const { return uint32_value; }
	template<>
	inline constexpr value& value::set<value_type::divert, uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::divert;
		return *this;
	}

	template<> struct value::ret<value_type::uint32> { using type = uint32_t; };
	template<> inline uint32_t value::get<value_type::uint32>() const { return uint32_value; }
	template<>
	inline constexpr value& value::set<value_type::uint32, uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::divert;
		return *this;
	}

	template<> struct value::ret<value_type::float32> { using type = float; };
	template<> inline float value::get<value_type::float32>() const { return float_value; }
	template<>
	inline constexpr value& value::set<value_type::float32, float>(float v) {
		float_value = v;
		_type = value_type::float32;
		return *this;
	}

	template<> struct value::ret<value_type::boolean> { using type = bool; };
	template<> inline bool value::get<value_type::boolean>() const { return bool_value; }
	template<>
	inline constexpr value& value::set<value_type::boolean, bool>(bool v) {
		bool_value = v;
		_type = value_type::boolean;
		return *this;
	}

	template<> struct value::ret<value_type::string> { using type = const char*; };
	template<> inline const char* value::get<value_type::string>() const { return str_value; }
	template<>
	inline constexpr value& value::set<value_type::string, const char*>(const char* v) {
		// TODO: decode allocw
		str_value = v;
		_type = value_type::string;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::string, const char*, bool>(const char* v, bool allocated) {
		// TODO use bool
		str_value = v;
		_type = value_type::string;
		return *this;
	}

	template<> struct value::ret<value_type::jump_marker> { using type = uint32_t; };
	template<> inline uint32_t value::get<value_type::jump_marker>() const { return uint32_value; }
	template<>
	inline constexpr value& value::set<value_type::jump_marker,uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::jump_marker;
		return *this;
	}

	template<> struct value::ret<value_type::thread_start> { using type = uint32_t; };
	template<> inline uint32_t value::get<value_type::thread_start>() const { return uint32_value; }
	template<>
	inline constexpr value& value::set<value_type::thread_start,uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::thread_start;
		return *this;
	}

	template<> struct value::ret<value_type::thread_end> { using type = uint32_t; };
	template<> inline uint32_t value::get<value_type::thread_end>() const { return uint32_value; }
	template<>
	inline constexpr value& value::set<value_type::thread_end, uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::thread_end;
		return *this;
	}

	template<>
	inline constexpr value& value::set<value_type::marker>() {
		_type = value_type::marker;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::glue>() {
		_type = value_type::glue;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::newline>() {
		_type = value_type::newline;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::func_start>() {
		_type = value_type::func_start;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::func_end>() {
		_type = value_type::func_end;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::null>() {
		_type = value_type::null;
		return *this;
	}

	template<>
	inline constexpr value& value::set<value_type::function_frame,uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::function_frame;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::tunnel_frame,uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::tunnel_frame;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::thread_frame,uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::thread_frame;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::tunnel_frame>() {
		_type = value_type::tunnel_frame;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::function_frame>() {
		_type = value_type::function_frame;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::thread_end>() {
		_type = value_type::thread_end;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::none>() {
		_type = value_type::none;
		return *this;
	}
	namespace values {
		static constexpr value marker = value{}.set<value_type::marker>();
		static constexpr value glue = value{}.set<value_type::glue>();
		static constexpr value newline = value{}.set<value_type::newline>();
		static constexpr value func_start = value{}.set<value_type::func_start>();
		static constexpr value func_end = value{}.set<value_type::func_end>();
		static constexpr value null = value{}.set<value_type::null>();
		static constexpr value tunnel_frame = value{}.set<value_type::tunnel_frame>();
		static constexpr value function_frame = value{}.set<value_type::function_frame>();
		static constexpr value thread_end = value{}.set<value_type::thread_end>();
	}
}
