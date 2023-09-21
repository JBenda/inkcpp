#pragma once

#include "list.h"
#include "story_ptr.h"
#include "system.h"

namespace ink::runtime
{
	class globals_interface;
	class runner_interface;
	class snapshot;

	using globals =  story_ptr<globals_interface>;
	using runner =  story_ptr<runner_interface>;
	using list = list_interface*;

	struct value {
		union {
			bool v_bool;
			uint32_t v_uint32;
			int32_t v_int32;
			const char* v_string;
			float v_float;
			list v_list;
		};
		enum class Type {
			Bool, Uint32, Int32, String, Float, List
		} type;
		value() : v_int32{0}, type{Type::Int32} {}
		value(bool v) : v_bool{v}, type{Type::Bool} {}
		value(uint32_t v) : v_uint32{v}, type{Type::Uint32} {}
		value(int32_t v) : v_int32{v}, type{Type::Int32} {}
		value(const char* v) : v_string{v}, type{Type::String} {}
		value(float v) : v_float{v}, type{Type::Float} {}
		value(list_interface* list) : v_list{list}, type{Type::List} {}
		value(const value& v) : type{v.type} {
			switch(type) {
				case Type::Bool:  v_bool = v.v_bool; break;
				case Type::Uint32: v_uint32 = v.v_uint32; break;
				case Type::Int32: v_int32 = v.v_int32; break;
				case Type::String: v_string = v.v_string; break;
				case Type::Float: v_float = v.v_float; break;
				case Type::List: v_list = v.v_list; break;
			}
		}

		template<Type Ty>
		const auto& get() const {
			static_assert(Ty != Ty, "No value getter for the selected type");
		}
	};

	template<>
	inline const auto& value::get<value::Type::Bool>() const {
		return v_bool;
	}
	template<>
	inline const auto& value::get<value::Type::Uint32>() const {
		return v_uint32;
	}
	template<>
	inline const auto& value::get<value::Type::Int32>() const {
		return v_int32;
	}
	template<>
	inline const auto& value::get<value::Type::String>() const {
		return v_string;
	}
	template<>
	inline const auto& value::get<value::Type::Float>() const {
		return v_float;
	}
	template<>
	inline const auto& value::get<value::Type::List>() const {
		return v_list;
	}
}
