#pragma once

#include "list.h"
#include "story_ptr.h"
#include "system.h"

namespace ink::runtime
{
class globals_interface;
class runner_interface;
class snapshot;

/** alias for an managed @ref ink::runtime::globals_interface pointer */
using globals = story_ptr<globals_interface>;
/** alias for an managed @ref ink::runtime::runner_interface pointer */
using runner  = story_ptr<runner_interface>;
/** alias for @ref ink::runtime::list_interface pointer */
using list    = list_interface*;

/** A Ink variable
 *
 * Used for accassing, writing and observing global variables
 * @ref ink::runtime::globals_interface::get()
 * @ref ink::runtime::globals_interface::set()
 * @ref ink::runtime::globals_interface::observe()
 *
 * and for the execution of extern functions
 * @ref ink::runtime::runner_interface::bind()
 */
struct value {
private:
	union {
		bool        v_bool;
		uint32_t    v_uint32;
		int32_t     v_int32;
		const char* v_string;
		float       v_float;
		list        v_list;
	};

public:
	/** Type labels for @ref ink::runtime::value  */
	enum class Type {
		Bool,   ///< containing bool
		Uint32, ///< containing uint32_t
		Int32,  ///< containing int32_t
		String, ///< contaning a const char*
		Float,  ///< containing a float
		List    ///< containing a @ref list_interface
	} type;   ///< Label of type currently contained in @ref value

	value()
	    : v_int32{0}
	    , type{Type::Int32}
	{
	}

	///@{
	/** Construct value from corresponding type */
	value(bool v)
	    : v_bool{v}
	    , type{Type::Bool}
	{
	}

	value(uint32_t v)
	    : v_uint32{v}
	    , type{Type::Uint32}
	{
	}

	value(int32_t v)
	    : v_int32{v}
	    , type{Type::Int32}
	{
	}

	value(const char* v)
	    : v_string{v}
	    , type{Type::String}
	{
	}

	value(float v)
	    : v_float{v}
	    , type{Type::Float}
	{
	}

	value(list_interface* list)
	    : v_list{list}
	    , type{Type::List}
	{
	}

	value(const value& v)
	    : type{v.type}
	{
		switch (type) {
			case Type::Bool: v_bool = v.v_bool; break;
			case Type::Uint32: v_uint32 = v.v_uint32; break;
			case Type::Int32: v_int32 = v.v_int32; break;
			case Type::String: v_string = v.v_string; break;
			case Type::Float: v_float = v.v_float; break;
			case Type::List: v_list = v.v_list; break;
		}
	}

	/// @}

	/** Get value to corresponding type
	 * @tparam Ty #Type label of type to get
	 * @attention behavior if undefined if Ty != value.type
	 */
	template<Type Ty>
	const auto& get() const
	{
		static_assert(Ty != Ty, "No value getter for the selected type");
	}
};

/** access #value::Type::Bool value */
template<>
inline const auto& value::get<value::Type::Bool>() const
{
	return v_bool;
}

/** access #value::Type::Uint32 value */
template<>
inline const auto& value::get<value::Type::Uint32>() const
{
	return v_uint32;
}

/** access #value::Type::Int32 value */
template<>
inline const auto& value::get<value::Type::Int32>() const
{
	return v_int32;
}

/** access #value::Type::String value */
template<>
inline const auto& value::get<value::Type::String>() const
{
	return v_string;
}

/** access #value::Type::Float value */
template<>
inline const auto& value::get<value::Type::Float>() const
{
	return v_float;
}

/** access #value::Type::List value */
template<>
inline const auto& value::get<value::Type::List>() const
{
	return v_list;
}
} // namespace ink::runtime
