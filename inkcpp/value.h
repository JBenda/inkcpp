#pragma once

/// The value class contains the information which type its hold and a small
/// piece of information to access the data.
/// use explicit getter and setter to make access more uniform.
/// define different value_types, and the mapping between type and data.

#include "system.h"
#include "../shared/private/command.h"
#include "list_table.h"
#include "tuple.hpp"

#ifdef INK_ENABLE_STL
#include <iosfwd>
#endif


namespace ink::runtime::internal {
	class basic_stream;

	/// different existing value_types
	enum class value_type {
		BEGIN, 						// To find the start of list
		none = BEGIN,               // no type -> invalid
		OP_BEGIN,					// first type to operate on
		divert = OP_BEGIN,          // divert to different story position
		PRINT_BEGIN,                // first printable value
		boolean = PRINT_BEGIN,      // boolean variable
		uint32,                     // 32bit unsigned integer variable
		int32,                      // 32bit integer variable
		float32,                    // 32bit floating point value 
		list,						// id of list in list_table
		list_flag,					// a single list flag
		string,                     // Pointer to string
		OP_END,                     // END of types where we can operate on
		value_pointer,				// a pointer to an value
		newline = OP_END,           // newline symbol
		PRINT_END,                  // END of printable values
		marker = PRINT_END,         // special marker (used in output stream)
		glue,                       // glue symbol
		func_start,                 // start of function marker
		func_end,                   // end of function marker
		null,                       // void value, for function returns
		tunnel_frame,               // return from tunnel
		function_frame,             // return from function
		thread_frame,               // return from thread
		thread_start,               // start of thread frame
		thread_end,                 // end of thread frame
		jump_marker                 // callstack jump
		};

	// add operator for value_type (to simplify usage templates).
	constexpr value_type operator+(value_type t, int i) {
		return static_cast<value_type>(static_cast<int>(t)+i);
	}
	// add operator for Command (to simplify usage in templates).
	constexpr Command operator+(Command c, int i) {
		return static_cast<Command>(static_cast<int>(c)+i);
	}


	struct string_type{
		constexpr string_type(const char* string, bool allocated)
			: str{string}, allocated{allocated}{}
		constexpr string_type(const char* string)
			: str{string}, allocated{true} {}
		operator const char*() const {
			return str;
		}
		const char* str;
		bool allocated;
	};

	class value;
	template<value_type ty, typename T, typename ENV>
	class redefine {
	public:
		redefine(const ENV&) {}
		value operator()(const T& lh, const T& rh);
	};

	/**
	 * @brief class to wrap stack value to common type.
	 */
	class value {
	public:
		/// help struct to determine cpp type which represent the value_type
		template<value_type> struct ret { using type = void; };

		constexpr value() : _type{value_type::none}, uint32_value{0}{}

		/// get value of the type (if possible)
		template<value_type ty>
		typename ret<ty>::type
		get() const { static_assert(ty != ty, "No getter for this type defined!"); }

		/// set value of type (if possible)
		template<value_type ty, typename ...Args>
		constexpr value& set(Args ...args) {
			static_assert(sizeof...(Args)!=sizeof...(Args), "No setter for this type defined!");
			return *this;
		}

		/// get type of value
		constexpr value_type type() const { return _type; }

		/// returns if type is printable (see value_type)
		constexpr bool printable() const {
			return _type >= value_type::PRINT_BEGIN && _type < value_type::PRINT_END;
		}

		friend basic_stream& operator<<(basic_stream& os, const value&);
		// friend basic_stream& operator>>(basic_stream& is, value&); // TODO: implement
#ifdef INK_ENABLE_STL
		/** write  value string to ostream
		 * @param lists may set to list_table if list serelasation needed
		 */
		std::ostream& write(std::ostream&, const list_table* lists = nullptr) const;
#endif

		/// execute the type exclusive overwrite function and return a new value with
		/// this new type
		template<typename ... T>
		value redefine(const value& oth, T& ... env) const {
			inkAssert(type() == oth.type());
			return redefine<value_type::OP_BEGIN, T...>(oth, {&env...});
		}

	private:
		template<value_type ty, typename ... T>
		value redefine(const value& oth, const tuple<T*...>& env) const {
			if constexpr ( ty == value_type::OP_END) {
				throw ink_exception("Can't redefine value with this type! (It is not an variable type!)");
			} else if (ty != type()) {
				return redefine<ty + 1>(oth, env);
			} else {
				return internal::redefine<ty, typename ret<ty>::type, tuple<T*...>>(env)(get<ty>(), oth.get<ty>());
			}
		}

		/// actual storage
		union {
			bool bool_value;
			int32_t int32_value;
			string_type string_value;
			uint32_t uint32_value;
			float float_value;
			struct {
				uint32_t jump;
				uint32_t thread_id;
			} jump;
			list_table::list list_value;
			list_flag list_flag_value;
			struct {
				uint32_t addr;
				bool eval; // was eval mode active in frame above
			} frame_value;
			struct {
				hash_t name;
				char ci;
			} pointer;
		};
		value_type _type;
	};

	template<value_type ty, typename T, typename ENV>
	value redefine<ty,T,ENV>::operator()(const T& lh, const T& rh) {
		return value{}.set<ty>(rh);
	}

	// define get and set for int32
	template<> struct value::ret<value_type::int32> { using type = int32_t; };
	template<> inline int32_t value::get<value_type::int32>() const { return int32_value; }
	template<>
	inline constexpr value& value::set<value_type::int32, int32_t>(int32_t v) {
		int32_value = v;
		_type = value_type::int32;
		return *this;
	}


	// define get and set for uint32
	template<> struct value::ret<value_type::uint32> { using type = uint32_t; };
	template<> inline uint32_t value::get<value_type::uint32>() const { return uint32_value; }
	template<>
	inline constexpr value& value::set<value_type::uint32, uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::uint32;
		return *this;
	}

	// define get and set for divert
	template<> struct value::ret<value_type::divert> { using type = uint32_t; };
	template<> inline uint32_t value::get<value_type::divert>() const { return uint32_value; }
	template<>
	inline constexpr value& value::set<value_type::divert, uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::divert;
		return *this;
	}

	// define get and set for float
	template<> struct value::ret<value_type::float32> { using type = float; };
	template<> inline float value::get<value_type::float32>() const { return float_value; }
	template<>
	inline constexpr value& value::set<value_type::float32, float>(float v) {
		float_value = v;
		_type = value_type::float32;
		return *this;
	}

	// define get and set for boolean
	template<> struct value::ret<value_type::boolean> { using type = bool; };
	template<> inline bool value::get<value_type::boolean>() const { return bool_value; }
	template<>
	inline constexpr value& value::set<value_type::boolean, bool>(bool v) {
		bool_value = v;
		_type = value_type::boolean;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::boolean, int>(int v) {
		bool_value = static_cast<bool>(v);
		_type = value_type::boolean;
		return *this;
	}

	// define get and set for list
	template<> struct value::ret<value_type::list> { using type = list_table::list; };
	template<> inline list_table::list value::get<value_type::list>() const { return list_value; }
	template<>
	inline constexpr value& value::set<value_type::list, list_table::list>(list_table::list list)
	{
		list_value = list;
		_type = value_type::list;
		return *this;
	}

	// define get and set for list_flag
	template<> struct value::ret<value_type::list_flag> { using type = list_flag; };
	template<> inline list_flag value::get<value_type::list_flag>() const { return list_flag_value; }
	template<>
	inline constexpr value& value::set<value_type::list_flag, list_flag>(list_flag flag)
	{
		list_flag_value = flag;
		_type = value_type::list_flag;
		return *this;
	}

	// define get and set for string
	template<> struct value::ret<value_type::string> { using type = string_type; };
	template<> inline string_type value::get<value_type::string>() const { return string_value; }
	template<>
	inline constexpr value& value::set<value_type::string, const char*>(const char* v) {
		string_value = {v};
		_type = value_type::string;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::string,char*>(char* v) {
		string_value = {v};
		_type = value_type::string;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::string, const char*, bool>(const char* v, bool allocated) {
		string_value = {v, allocated};
		_type = value_type::string;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::string, char*, bool>( char* v, bool allocated) {
		string_value = {v, allocated};
		_type = value_type::string;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::string, string_type>(
			string_type str) {
		string_value = str;
		_type = value_type::string;
		return *this;
	}

	// define get and set for pointer
	template<> struct value::ret<value_type::value_pointer> { using type = decltype(value::pointer); }; 
	template<> inline value::ret<value_type::value_pointer>::type value::get<value_type::value_pointer>() const
	{ return pointer; }
	template<>
	inline constexpr value& value::set<value_type::value_pointer,hash_t,int>(hash_t name, int ci) {
		_type = value_type::value_pointer;
		pointer.name = name;
		pointer.ci = ci;
		return *this;
	}

	// define getter and setter for jump_marker
	template<> struct value::ret<value_type::jump_marker> { using type = decltype(value::jump); };
	template<> inline value::ret<value_type::jump_marker>::type value::get<value_type::jump_marker>() const { return jump; }
	template<>
	inline constexpr value& value::set<value_type::jump_marker,decltype(value::jump)>(decltype(value::jump) v) {
		jump = v;
		_type = value_type::jump_marker;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::jump_marker,uint32_t,uint32_t>(uint32_t v, uint32_t j) {
		jump.jump = v;
		jump.thread_id = j;
		_type = value_type::jump_marker;
		return *this;
	}

	// define getter and setter for thread_start
	template<> struct value::ret<value_type::thread_start> { using type = decltype(value::jump); };
	template<> inline value::ret<value_type::thread_start>::type value::get<value_type::thread_start>() const { return jump; }
	template<>
	inline constexpr value& value::set<value_type::thread_start,decltype(value::jump)>(decltype(value::jump) v)
	{
		jump = v;
		_type = value_type::thread_start;
		return *this;
	}
	template<>
	inline constexpr value& value::set<value_type::thread_start,uint32_t,uint32_t>(uint32_t v, uint32_t j) {
		jump.jump = v;
		jump.thread_id = j;
		_type = value_type::thread_start;
		return *this;
	}

	// define getter and setter for thread_end
	template<> struct value::ret<value_type::thread_end> { using type = uint32_t; };
	template<> inline uint32_t value::get<value_type::thread_end>() const { return uint32_value; }
	template<>
	inline constexpr value& value::set<value_type::thread_end, uint32_t>(uint32_t v) {
		uint32_value = v;
		_type = value_type::thread_end;
		return *this;
	}

	// define setter for values without storage
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
	template<> struct value::ret<value_type::newline> { using type = const char*; };
	template<>
	inline const char* value::get<value_type::newline>() const {
		static const char line_break[] = "\n";
		return line_break;
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
	inline constexpr value& value::set<value_type::none>() {
		_type = value_type::none;
		return *this;
	}

	// getter and setter for different frame types
	// FIXME: the getter are not used?
	template<> struct value::ret<value_type::function_frame>{ using type = decltype(frame_value); };
	template<> inline typename value::ret<value_type::function_frame>::type value::get<value_type::function_frame>() const { return frame_value; }
	template<>
	inline constexpr value& value::set<value_type::function_frame,uint32_t>(uint32_t v, bool evalOn) {
		frame_value.addr = v;
		frame_value.eval = evalOn;
		_type = value_type::function_frame;
		return *this;
	}
	// FIXME: the getter are not used?
	/* template<> struct value::ret<value_type::tunnel_frame>{ using type = uint32_t; }; */
	/* template<> inline uint32_t value::get<value_type::tunnel_frame>() const { return uint32_value; } */
	template<>
	inline constexpr value& value::set<value_type::tunnel_frame,uint32_t>(uint32_t v, bool evalOn) {
		frame_value.addr = v;
		frame_value.eval = evalOn;
		_type = value_type::tunnel_frame;
		return *this;
	}
	// FIXME: the getter are not used?
	/* template<> struct value::ret<value_type::thread_frame>{ using type = uint32_t; }; */
	/* template<> inline uint32_t value::get<value_type::thread_frame>() const { return uint32_value; } */
	template<>
	inline constexpr value& value::set<value_type::thread_frame,uint32_t>(uint32_t v, bool evalOn) {
		frame_value.addr = v;
		frame_value.eval = evalOn;
		_type = value_type::thread_frame;
		return *this;
	}

	// static constexpr instantiations of flag values
	namespace values {
		static constexpr value marker = value{}.set<value_type::marker>();
		static constexpr value glue = value{}.set<value_type::glue>();
		static constexpr value newline = value{}.set<value_type::newline>();
		static constexpr value func_start = value{}.set<value_type::func_start>();
		static constexpr value func_end = value{}.set<value_type::func_end>();
		static constexpr value null = value{}.set<value_type::null>();
	}
}
