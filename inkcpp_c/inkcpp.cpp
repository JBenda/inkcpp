#include "inkcpp.h"
#include "list.h"
#include "system.h"

#include <stdatomic.h>
#include <story.h>
#include <snapshot.h>
#include <globals.h>
#include <runner.h>
#include <choice.h>

using namespace ink::runtime;

InkValue inkvar_to_c(value& val)
{
	switch (val.type) {
		case value::Type::Bool:
			return InkValue{
			    .bool_v = val.get<value::Type::Bool>(),
			    .type   = InkValue::ValueTypeBool,
			};
		case value::Type::Uint32:
			return InkValue{
			    .uint32_v = val.get<value::Type::Uint32>(),
			    .type     = InkValue::ValueTypeUint32,
			};
		case value::Type::Int32:
			return InkValue{
			    .int32_v = val.get<value::Type::Int32>(),
			    .type    = InkValue::ValueTypeInt32,
			};
		case value::Type::String:
			return InkValue{
			    .string_v = val.get<value::Type::String>(),
			    .type     = InkValue::ValueTypeString,
			};
		case value::Type::Float:
			return InkValue{
			    .float_v = val.get<value::Type::Float>(),
			    .type    = InkValue::ValueTypeFloat,
			};
		case value::Type::List:
			return InkValue{
			    .list_v = reinterpret_cast<HInkList*>(val.get<value::Type::List>()),
			    .type   = InkValue::ValueTypeList,
			};
	}
	inkFail("Undefined value type can not be translated");
	return InkValue{};
}

value inkvar_from_c(InkValue& val)
{
	switch (val.type) {
		case InkValue::ValueTypeBool: return value(val.bool_v);
		case InkValue::ValueTypeUint32: return value(val.uint32_v);
		case InkValue::ValueTypeInt32: return value(val.int32_v);
		case InkValue::ValueTypeString: return value(val.string_v);
		case InkValue::ValueTypeFloat: return value(val.float_v);
		case InkValue::ValueTypeList: return value(val.list_v);
	}
	inkFail("Undefined value type can not be translated");
	return value{};
}

extern "C" {
	HInkSnapshot* ink_snapshot_from_file(const char* filename)
	{
		return reinterpret_cast<HInkSnapshot*>(snapshot::from_file(filename));
	}

	int ink_snapshot_num_runners(const HInkSnapshot* self)
	{
		return reinterpret_cast<const snapshot*>(self)->num_runners();
	}

	void ink_snapshot_write_to_file(const HInkSnapshot* self, const char* filename)
	{
		reinterpret_cast<const snapshot*>(self)->write_to_file(filename);
	}

	const char* ink_choice_text(const HInkChoice* self)
	{
		return reinterpret_cast<const choice*>(self)->text();
	}

	int ink_choice_num_tags(const HInkChoice* self)
	{
		return reinterpret_cast<const choice*>(self)->num_tags();
	}

	const char* ink_choice_get_tag(const HInkChoice* self, int tag_id)
	{
		return reinterpret_cast<const choice*>(self)->get_tag(tag_id);
	}

	void ink_list_add(HInkList* self, const char* flag_name)
	{
		reinterpret_cast<list>(self)->add(flag_name);
	}

	void ink_list_remove(HInkList* self, const char* flag_name)
	{
		reinterpret_cast<list>(self)->remove(flag_name);
	}

	int ink_list_contains(const HInkList* self, const char* flag_name)
	{
		return reinterpret_cast<const list_interface*>(self)->contains(flag_name);
	}

	void ink_list_flags(const HInkList* self, InkListIter* iter)
	{
		list_interface::iterator itr = reinterpret_cast<const list_interface*>(self)->begin();
		*iter                        = InkListIter{
		                           ._data        = &itr._list,
		                           ._i           = itr._i,
		                           ._single_list = itr._one_list_iterator,
		                           .flag_name    = itr._flag_name,
		                           .list_name    = itr._list_name,
    };
	}

	void ink_list_flags_from(const HInkList* self, const char* list_name, InkListIter* iter)
	{
		list_interface::iterator itr = reinterpret_cast<const list_interface*>(self)->begin(list_name);
		*iter                        = InkListIter{
		                           ._data        = &itr._list,
		                           ._i           = itr._i,
		                           ._single_list = itr._one_list_iterator,
		                           .flag_name    = itr._flag_name,
		                           .list_name    = itr._list_name,
    };
	}

	int ink_list_iter_next(InkListIter* self)
	{
		list_interface::iterator itr(
		    self->flag_name, *reinterpret_cast<const list_interface*>(self->_data), self->_i,
		    self->_single_list
		);
		++itr;
		return itr != itr._list.end();
	}

	void ink_runner_delete(HInkRunner* self) { delete reinterpret_cast<const runner*>(self); }

	HInkSnapshot* ink_runner_create_snapshot(const HInkRunner* self)
	{
		return reinterpret_cast<HInkSnapshot*>(
		    reinterpret_cast<const runner*>(self)->get()->create_snapshot()
		);
	}

	int ink_runner_can_continue(const HInkRunner* self)
	{
		return reinterpret_cast<const runner*>(self)->get()->can_continue();
	}

	const char* ink_runner_get_line(HInkRunner* self)
	{
		return reinterpret_cast<runner*>(self)->get()->getline_alloc();
	}

	int ink_runner_get_num_tags(const HInkRunner* self)
	{
		return reinterpret_cast<const runner*>(self)->get()->num_tags();
	}

	const char* ink_runner_tag(const HInkRunner* self, int tag_id)
	{
		return reinterpret_cast<const runner*>(self)->get()->get_tag(tag_id);
	}

	int ink_runner_num_choices(const HInkRunner* self)
	{
		return reinterpret_cast<const runner*>(self)->get()->num_choices();
	}

	const HInkChoice* ink_runner_get_choice(const HInkRunner* self, int choice_id)
	{
		return reinterpret_cast<const HInkChoice*>(
		    reinterpret_cast<const runner*>(self)->get()->get_choice(choice_id)
		);
	}

	void ink_runner_choose(HInkRunner* self, int choice_id)
	{
		return reinterpret_cast<runner*>(self)->get()->choose(choice_id);
	}

	void ink_runner_bind_void(
	    HInkRunner* self, const char* function_name, InkExternalFunctionVoid callback
	)
	{
		static_assert(sizeof(ink::runtime::value) >= sizeof(InkValue));
		return reinterpret_cast<runner*>(self)->get()->bind(
		    function_name,
		    [callback](size_t len, const value* vals) {
			    InkValue* c_vals = reinterpret_cast<InkValue*>(const_cast<value*>(vals));
			    int       c_len  = len;
			    for (int i = 0; i < c_len; ++i) {
				    c_vals[i] = inkvar_to_c(const_cast<value&>(vals[i]));
			    }
			    callback(c_len, c_vals);
		    }
		);
	}

	void ink_runner_bind(HInkRunner* self, const char* function_name, InkExternalFunction callback)
	{
		static_assert(sizeof(ink::runtime::value) >= sizeof(InkValue));
		return reinterpret_cast<runner*>(self)->get()->bind(
		    function_name,
		    [callback](size_t len, const value* vals) -> value {
			    InkValue* c_vals = reinterpret_cast<InkValue*>(const_cast<value*>(vals));
			    int       c_len  = len;
			    for (int i = 0; i < c_len; ++i) {
				    c_vals[i] = inkvar_to_c(const_cast<value&>(vals[i]));
			    }
			    InkValue res = callback(c_len, c_vals);
			    return inkvar_from_c(res);
		    }
		);
	}

	HInkStory* ink_story_from_file(const char* filename)
	{
		return reinterpret_cast<HInkStory*>(story::from_file(filename));
	}

	void ink_story_delete(HInkStory* self) { delete reinterpret_cast<story*>(self); }

	HInkRunner* ink_story_new_runner(HInkStory* self, HInkGlobals* global_store)
	{
		runner* res = new runner(
		    global_store
		        ? reinterpret_cast<story*>(self)->new_runner(*reinterpret_cast<globals*>(global_store))
		        : reinterpret_cast<story*>(self)->new_runner()
		);
		return reinterpret_cast<HInkRunner*>(res);
	}
}
