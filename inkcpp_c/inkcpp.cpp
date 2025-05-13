#include "inkcpp.h"
#include "list.h"
#include "system.h"
#include "types.h"

#include <cstring>

#include <memory>
#include <story.h>
#include <snapshot.h>
#include <globals.h>
#include <runner.h>
#include <choice.h>
#include <compiler.h>

using namespace ink::runtime;

InkValue inkvar_to_c(value& val)
{
	InkValue value{};
	switch (val.type) {
		case value::Type::Bool:
			value.bool_v = val.get<value::Type::Bool>();
			value.type   = InkValue::ValueTypeBool;
			return value;
		case value::Type::Uint32:
			value.uint32_v = val.get<value::Type::Uint32>();
			value.type     = InkValue::ValueTypeUint32;
			return value;
		case value::Type::Int32:
			value.int32_v = val.get<value::Type::Int32>();
			value.type    = InkValue::ValueTypeInt32;
			return value;
		case value::Type::String:
			value.string_v = val.get<value::Type::String>();
			value.type     = InkValue::ValueTypeString;
			return value;
		case value::Type::Float:
			value.float_v = val.get<value::Type::Float>();
			value.type    = InkValue::ValueTypeFloat;
			return value;
		case value::Type::List:
			value.list_v = reinterpret_cast<HInkList*>(val.get<value::Type::List>());
			value.type   = InkValue::ValueTypeList;
			return value;
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
		case InkValue::ValueTypeList: return value(reinterpret_cast<list_interface*>(val.list_v));
		case InkValue::ValueTypeNone: break;
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

	int ink_list_flags(const HInkList* self, InkListIter* iter)
	{
		list_interface::iterator itr = reinterpret_cast<const list_interface*>(self)->begin();
		*iter                        = InkListIter{
        &itr._list, itr._i, itr._one_list_iterator, itr._flag_name, itr._list_name,
    };
		return itr != reinterpret_cast<const list_interface*>(self)->end();
	}

	int ink_list_flags_from(const HInkList* self, const char* list_name, InkListIter* iter)
	{
		list_interface::iterator itr = reinterpret_cast<const list_interface*>(self)->begin(list_name);
		*iter                        = InkListIter{
        &itr._list, itr._i, itr._one_list_iterator, itr._flag_name, itr._list_name,
    };
		return itr != reinterpret_cast<const list_interface*>(self)->end();
	}

	int ink_list_iter_next(InkListIter* self)
	{
		list_interface::iterator itr(
		    self->flag_name, *reinterpret_cast<const list_interface*>(self->_data), self->_i,
		    self->_single_list
		);
		++itr;
		self->flag_name = itr._flag_name;
		self->list_name = itr._list_name;
		self->_i        = itr._i;
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

	int ink_runner_num_tags(const HInkRunner* self)
	{
		return reinterpret_cast<const runner*>(self)->get()->num_tags();
	}

	const char* ink_runner_tag(const HInkRunner* self, int tag_id)
	{
		return reinterpret_cast<const runner*>(self)->get()->get_tag(tag_id);
	}

	int ink_runner_num_knot_tags(const HInkRunner* self)
	{
		return reinterpret_cast<const runner*>(self)->get()->num_knot_tags();
	}

	ink_hash_t ink_runner_current_knot(const HInkRunner* self)
	{
		return reinterpret_cast<const runner*>(self)->get()->get_current_knot();
	}

	bool ink_runner_move_to(HInkRunner* self, ink_hash_t path)
	{
		return reinterpret_cast<runner*>(self)->get()->move_to(path);
	}

	ink_hash_t ink_hash_string(const char* str) { return ink::hash_string(str); }

	const char* ink_runner_knot_tag(const HInkRunner* self, int tag_id)
	{
		return reinterpret_cast<const runner*>(self)->get()->get_knot_tag(tag_id);
	}

	int ink_runner_num_global_tags(const HInkRunner* self)
	{
		return reinterpret_cast<const runner*>(self)->get()->num_global_tags();
	}

	const char* ink_runner_global_tag(const HInkRunner* self, int tag_id)
	{
		return reinterpret_cast<const runner*>(self)->get()->get_global_tag(tag_id);
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
	    HInkRunner* self, const char* function_name, InkExternalFunctionVoid callback,
	    int lookaheadSafe
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
		    },
		    lookaheadSafe
		);
	}

	void ink_runner_bind(
	    HInkRunner* self, const char* function_name, InkExternalFunction callback, int lookaheadSafe
	)
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
		    },
		    lookaheadSafe
		);
	}

	void ink_globals_delete(HInkGlobals* self) { delete reinterpret_cast<globals*>(self); }

	HInkSnapshot* ink_globals_create_snapshot(const HInkGlobals* self)
	{
		return reinterpret_cast<HInkSnapshot*>(
		    reinterpret_cast<const globals*>(self)->get()->create_snapshot()
		);
	}

	constexpr InkValue ink_value_none()
	{
		InkValue value{};
		value.type = InkValue::Type::ValueTypeNone;
		return value;
	}

	void ink_globals_observe(HInkGlobals* self, const char* variable_name, InkObserver observer)
	{
		reinterpret_cast<globals*>(self)->get()->observe(
		    variable_name,
		    [observer](value new_value, ink::optional<value> old_value) {
			    observer(
			        inkvar_to_c(new_value),
			        old_value.has_value() ? inkvar_to_c(old_value.value()) : ink_value_none()
			    );
		    }
		);
	}

	InkValue ink_globals_get(const HInkGlobals* self, const char* variable_name)
	{
		ink::optional<value> o_val
		    = reinterpret_cast<const globals*>(self)->get()->get<value>(variable_name);
		if (! o_val.has_value()) {
			return ink_value_none();
		} else {
			return inkvar_to_c(o_val.value());
		}
	}

	int ink_globals_set(HInkGlobals* self, const char* variable_name, InkValue val)
	{
		return reinterpret_cast<globals*>(self)->get()->set(variable_name, inkvar_from_c(val));
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

	HInkRunner* ink_story_new_runner_from_snapshot(
	    HInkStory* self, const HInkSnapshot* snapshot, HInkGlobals* global_store, int runner_id
	)
	{
		const ink::runtime::snapshot& snap = *reinterpret_cast<const ink::runtime::snapshot*>(snapshot);
		runner*                       res  = new runner(
        global_store
            ? reinterpret_cast<story*>(self)->new_runner_from_snapshot(
                  snap, *reinterpret_cast<globals*>(global_store), runner_id
              )
            : reinterpret_cast<story*>(self)->new_runner_from_snapshot(snap, nullptr, runner_id)
		);
		return reinterpret_cast<HInkRunner*>(res);
	}

	HInkGlobals* ink_story_new_globals(HInkStory* self)
	{
		return reinterpret_cast<HInkGlobals*>(new globals(reinterpret_cast<story*>(self)->new_globals())
		);
	}

	HInkGlobals* ink_story_new_globals_from_snapshot(HInkStory* self, const HInkSnapshot* snap)
	{
		return reinterpret_cast<HInkGlobals*>(
		    new globals(reinterpret_cast<story*>(self)->new_globals_from_snapshot(
		        *reinterpret_cast<const snapshot*>(snap)
		    ))
		);
	}

	void ink_compile_json(const char* input_filename, const char* output_filename, const char** error)
	{
		ink::compiler::compilation_results result;
		ink::compiler::run(input_filename, output_filename, &result);
		if (error != nullptr && ! result.errors.empty() || ! result.warnings.empty()) {
			std::string str{};
			for (auto& warn : result.warnings) {
				str += "WARNING: " + warn + '\n';
			}
			for (auto& err : result.errors) {
				str += "ERROR: " + err + '\n';
			}
			*error = strdup(str.c_str());
		}
	}
}
