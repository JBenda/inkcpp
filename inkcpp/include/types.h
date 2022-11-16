#pragma once

#include "story_ptr.h"

namespace ink
{

}

namespace ink::runtime
{
	class globals_interface;
	class runner_interface;
	class snapshot;

	typedef story_ptr<globals_interface> globals;
	typedef story_ptr<runner_interface> runner;
	
	struct value {
		union {
			bool v_bool;
			uint32_t v_uint32;
			int32_t v_int32;
			const char* v_string;
			float v_float;
		};
		enum class Type {
			Bool, Uint32, Int32, String, Float
		} type;
		value() : v_uint32{0}, type{Type::Int32} {}
		value(bool v) : v_bool{v}, type{Type::Bool} {}
		value(uint32_t v) : v_uint32{v}, type{Type::Uint32} {}
		value(int32_t v) : v_int32{v}, type{Type::Int32} {}
		value(const char* v) : v_string{v}, type{Type::String} {}
		value(float v) : v_float{v}, type{Type::Float} {}
	};
}
