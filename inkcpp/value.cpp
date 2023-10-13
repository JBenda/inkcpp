#include "value.h"

#include "list_impl.h"
#include "output.h"
#include "list_table.h"
#include "string_utils.h"
#include "string_table.h"
#include "system.h"

namespace ink::runtime::internal
{
	
	template<value_type = value_type::OP_BEGIN>
	bool truthy_impl(const value& v, const list_table& lists);

	template<>
	bool truthy_impl<value_type::OP_END>(const value& v, const list_table& lists) {
		inkFail("Type was not found in operational types or it has no conversion to boolean");
		return false;
	}
	
	template<>
	bool truthy_impl<value_type::string>(const value& v, const list_table& lists) {
		if(v.type() == value_type::string) {
			// if string is not empty
			return *v.get<value_type::string>().str != 0;
		} else {
			return truthy_impl<value_type::string+1>(v, lists);
		}
	}
	
	template<>
	bool truthy_impl<value_type::list_flag>(const value& v, const list_table& lists) {
		// if list is not empty -> valid flag -> filled list
		if(v.type() == value_type::list_flag) {
			auto flag = v.get<value_type::list_flag>();
			return flag != null_flag && flag != empty_flag;
		} else {
			return truthy_impl<value_type::list_flag+1>(v, lists);
		}
	}
	
	template<>
	bool truthy_impl<value_type::list>(const value& v, const list_table& lists) {
		// if list is not empty
		if(v.type() == value_type::list) {
			return lists.count(v.get<value_type::list>()) > 0;
		} else {
			return truthy_impl<value_type::list +1>(v, lists);
		}
	}
	
	template<>
	bool truthy_impl<value_type::float32>(const value& v, const list_table& lists) {
		if (v.type() == value_type::float32) {
			return v.get<value_type::float32>() != 0.0f;
		} else {
			return truthy_impl<value_type::float32+1>(v, lists);
		}
	}
	template<>
	bool truthy_impl<value_type::int32>(const value& v, const list_table& lists) {
		if(v.type() == value_type::int32) {
			return v.get<value_type::int32>() != 0;
		} else {
			return truthy_impl<value_type::int32+1>(v, lists);
		}
	}
	
	template<>
	bool truthy_impl<value_type::uint32>(const value& v, const list_table& lists) {
		if (v.type() == value_type::uint32) {
			return v.get<value_type::uint32>() != 0;
		} else {
			return truthy_impl<value_type::uint32+1>(v, lists);
		}
	}
	
	template<>
	bool truthy_impl<value_type::boolean>(const value& v, const list_table& lists) {
		if(v.type() == value_type::boolean) {
			return v.get<value_type::boolean>();
		} else {
			return truthy_impl<value_type::boolean + 1>(v, lists);
		}
	}

	template<>
	bool truthy_impl<value_type::divert>(const value& v, const list_table& lists) {
		if (v.type() == value_type::divert) {
			inkFail("Divert can not be evaluated to boolean");
			return false;
		} else {
			return truthy_impl<value_type::divert + 1>(v, lists);
		}
	}

	bool value::truthy(const list_table& lists) const {
		return truthy_impl(*this, lists);
	}
	

	
#ifdef INK_ENABLE_STL
	template<value_type ty = value_type::PRINT_BEGIN>
	void append(std::ostream& os, const value& val, const list_table* lists) {
		if constexpr (ty != value_type::PRINT_END) {
			if (ty == val.type()) {
				os << val.get<ty>();
			} else {
				append<ty+1>(os, val, lists);
			}
		}
	}

	template<>
	void append<value_type::list_flag>(std::ostream& os, const value& val,
			const list_table* lists) {
		if (val.type() == value_type::list_flag) {
			inkAssert(lists, "to stringify lists, we need a list_table");
			os << lists->toString(val.get<value_type::list_flag>());
		} else {
			append<value_type::list_flag + 1>(os, val, lists);
		}
	}
	template<>
	void append<value_type::list>(std::ostream& os, const value& val,
			const list_table* lists) {
		if (val.type() == value_type::list) {
			inkAssert(lists, "to stringify lists, we need a list_table");
			lists->write(os, val.get<value_type::list>());
		} else {
			append<value_type::list +1>(os, val, lists);
		}
	}
	template<>
	void append<value_type::float32>(std::ostream& os, const value& val, const list_table* lists) {
		if(val.type() == value_type::float32) {
			char number[32];
			ink::runtime::internal::toStr(number, 32, val.get<value_type::float32>());
			os << number;
		} else {
			append<value_type::float32+1>(os, val, lists);
		}
	}
	template<>
	void append<value_type::boolean>(std::ostream& os, const value& val, const list_table* lists) {
		if(val.type() == value_type::boolean) {
			os << (val.get<value_type::boolean>() ? "true" : "false");
		} else {
			append<value_type::boolean + 1>(os, val, lists);
		}
	}

	std::ostream& value::write(std::ostream& os, const list_table* lists) const {
		if (type() < value_type::PRINT_BEGIN || type() >= value_type::PRINT_END) {
			throw ink_exception("printing this type is not supported");
		}
		append(os, *this, lists);
		return os;
	}
#endif

	basic_stream& operator<<(basic_stream& os, const value& val) {
		os.append(val);
		return os;
	}
	
	value::value(const ink::runtime::value& val) : value() {
		using types = ink::runtime::value::Type; 
		switch (val.type) {
			case types::Bool:
				set<value_type::boolean>(val.v_bool);
				break;
			case types::Uint32:
				set<value_type::uint32>(val.v_uint32);
				break;
			case types::Int32:
				set<value_type::int32>(val.v_int32);
				break;
			case types::String:
				set<value_type::string>(val.v_string);
				break;
			case types::Float:
				set<value_type::float32>(val.v_float);
				break;
			case types::List:
				set<value_type::list>(list_table::list{static_cast<list_impl*>(val.v_list)->get_lid()});
		}
	}

	bool value::set( const ink::runtime::value& val ) {
		auto var = value( val );
		if ( type() == value_type::none || var.type() == type() ) {
			*this = var;
			return true;
		}
		return false;
	}

	ink::runtime::value value::to_interface_value(list_table& table) const {
		using val = ink::runtime::value;
		if(type() == value_type::boolean) { return val(get<value_type::boolean>()); }
		else if(type() == value_type::uint32) { return val(get<value_type::uint32>()); }
		else if(type() == value_type::int32) { return val(get<value_type::int32>()); }
		else if(type() == value_type::string) { return val(get<value_type::string>().str); }
		else if(type() == value_type::float32) { return val(get<value_type::float32>()); }
		else if(type() == value_type::list_flag) { 
			auto v = table.create();
			v = table.add(v, get<value_type::list_flag>());
			return val(table.handout_list(v));
		} else if(type() == value_type::list) { 
			return val(table.handout_list(get<value_type::list>()));
		}
		inkFail("No valid type to convert to interface value!");
		return val();
	}

	size_t value::snap(unsigned char* data, const snapper& snapper) const
	{
		unsigned char* ptr = data;
		bool should_write = data != nullptr;
		ptr = snap_write(ptr, _type, should_write );
		if (_type == value_type::string) {
			unsigned char buf[max_value_size];
			string_type* res = reinterpret_cast<string_type*>(buf);
			auto str = get<value_type::string>();
			res->allocated = str.allocated;
			if (str.allocated) {
				res->str = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(snapper.strings.get_id(str.str)));
			} else {
				res->str = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(str.str - snapper.story_string_table));
			}
			ptr = snap_write(ptr, buf, should_write );
		} else {
			// TODO more space efficent?
			ptr = snap_write(ptr, &bool_value, max_value_size, should_write );
		}
		return ptr - data;
	}
	const unsigned char* value::snap_load(const unsigned char* ptr, const loader& loader)
	{
		ptr = snap_read(ptr, _type);
		ptr = snap_read(ptr, &bool_value, max_value_size);
		if(_type == value_type::string) {
			if(string_value.allocated) {
				string_value.str = loader.string_table[(std::uintptr_t)(string_value.str)];
			} else {
				string_value.str = loader.story_string_table +(std::uintptr_t)(string_value.str);
			}
		}
		return ptr;
	}
}
