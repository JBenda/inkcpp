/// implements operations on lists

#include "stack.h"
#include "value.h"
#include "operations.h"
#include "list_table.h"

#define call4_Wrap(OP, RET, FUN) call4_Wrap_diff(OP, RET, RET, RET, RET, FUN)
#define call4_Wrap_diff(OP, RET0, RET1, RET2, RET3, FUN) \
	void operation<Command::OP, value_type::list, void>::operator()(\
			basic_eval_stack& stack, value* vals)\
	{\
		call4(RET0, RET1, RET2, RET3, FUN);\
	}

#define call4(RET0, RET1, RET2, RET3, FUN) \
	if (vals[0].type() == value_type::list_flag) { \
		if(vals[1].type() == value_type::list) { \
			stack.push(value{}.set<value_type::RET0>( \
				_list_table.FUN( \
					vals[0].get<value_type::list_flag>(), \
					vals[1].get<value_type::list>() \
				) \
			)); \
		} else {\
			inkAssert(vals[1].type()==value_type::list_flag, "list operation was called but second argument is not a list or list_flag");\
			stack.push(value{}.set<value_type::RET1>( \
					_list_table.FUN( \
						vals[0].get<value_type::list_flag>(), \
						vals[1].get<value_type::list_flag>() \
					)\
			)); \
		} \
	} else { \
		inkAssert(vals[0].type() == value_type::list, "list operation was called but first argument is not a list or a list_flag!"); \
		if(vals[1].type() == value_type::list) { \
			stack.push(value{}.set<value_type::RET2>( \
				_list_table.FUN( \
					vals[0].get<value_type::list>(), \
					vals[1].get<value_type::list>() \
				) \
			)); \
		} else {\
			inkAssert(vals[1].type()==value_type::list_flag, "list operation was called but second argument ist not a list or list_flag!");\
			stack.push(value{}.set<value_type::RET3>( \
					_list_table.FUN( \
						vals[0].get<value_type::list>(), \
						vals[1].get<value_type::list_flag>() \
					)\
			)); \
		} \
	}

#define call2(OP, RET0, RET1, FUN) \
	void operation<Command::OP, value_type::list, void>::operator()( \
			basic_eval_stack& stack, value* vals)\
	{\
		stack.push(value{}.set<value_type::RET0>(\
			_list_table.FUN(vals[0].get<value_type::list>())\
		));\
	}\
	void operation<Command::OP, value_type::list_flag, void>::operator()(\
			basic_eval_stack& stack, value* vals)\
	{\
		stack.push(value{}.set<value_type::RET1>(\
			_list_table.FUN(vals[0].get<value_type::list_flag>())\
		));\
	}

namespace ink::runtime::internal {
	void operation<Command::ADD, value_type::list, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		if(vals[1].type() == value_type::int32
				|| vals[1].type() == value_type::uint32)
		{
			int i = vals[1].type() == value_type::int32
				? vals[1].get<value_type::int32>()
				: static_cast<int>(vals[1].get<value_type::uint32>());
			inkAssert(vals[0].type() == value_type::list, "try to use list add function but value is not of type list");
			stack.push(value{}.set<value_type::list>(
				_list_table.add(vals[0].get<value_type::list>(), i)
			));
		} else {
			call4(list, list, list, list, add);
		}
	}
	void operation<Command::ADD, value_type::list_flag, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		inkAssert(vals[0].type() == value_type::list_flag, "try to use add function with list_flag results but first argument is not a list_flag!");
		inkAssert(vals[1].type() == value_type::int32
				|| vals[1].type() == value_type::uint32,
				"try modify a list flag with a non intiger type!");
		int i = vals[1].type() == value_type::int32
			? vals[1].get<value_type::int32>()
			: static_cast<int>(vals[1].get<value_type::uint32>());
		stack.push(value{}.set<value_type::list_flag>(
			_list_table.add(vals[0].get<value_type::list_flag>(), i)
		));
	}

	void operation<Command::SUBTRACT, value_type::list, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		if(vals[1].type() == value_type::int32
				|| vals[1].type() == value_type::uint32)
		{
			int i = vals[1].type() == value_type::int32
				? vals[1].get<value_type::int32>()
				: vals[1].get<value_type::uint32>();
			inkAssert(vals[0].type() == value_type::list, "A in list resulting subtraction needs at leas one list as argument!");
			stack.push(value{}.set<value_type::list>(
				_list_table.sub(vals[0].get<value_type::list>(), i)
			));
		} else {
			call4(list_flag, list_flag, list, list, sub);
		}
	}
	void operation<Command::SUBTRACT, value_type::list_flag, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		inkAssert(vals[0].type() == value_type::list_flag, "subtraction resulting in list_flag needs a list_flag as first arguments!");
		inkAssert(vals[1].type() == value_type::int32
				|| vals[1].type() == value_type::uint32,
				"Try to subtract non integer value from list_flag.");
		int i = vals[1].type() == value_type::int32
			? vals[1].get<value_type::int32>()
			: vals[1].get<value_type::uint32>();
		stack.push(value{}.set<value_type::list_flag>(
			_list_table.sub(vals[0].get<value_type::list_flag>(), i)
		));
	}

	void operation<Command::INTERSECTION, value_type::list, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		call4(list_flag, list_flag, list, list_flag, intersect);
	}

	call2(LIST_COUNT, int32, int32, count);
	call2(LIST_MIN, list_flag, list_flag, min);
	call2(LIST_MAX, list_flag, list_flag, max);
	call2(LIST_ALL, list, list, all);
	call2(LIST_INVERT, list, list, invert);

	call4_Wrap(LESS_THAN, boolean, less);
	call4_Wrap(LESS_THAN_EQUALS, boolean, less_equal);
	call4_Wrap(GREATER_THAN, boolean, greater);
	call4_Wrap(GREATER_THAN_EQUALS, boolean, greater_equal);
	call4_Wrap(IS_EQUAL, boolean, equal);
	call4_Wrap(NOT_EQUAL, boolean, not_equal);
	call4_Wrap(HAS, boolean, has);
	call4_Wrap(HASNT, boolean, hasnt);


	void operation<Command::lrnd, value_type::list, void>::operator()( 
			basic_eval_stack& stack, value* vals)
	{
		stack.push(value{}.set<value_type::list_flag>(
			_list_table.lrnd(vals[0].get<value_type::list>(), _prng)
		));
	}
	void operation<Command::lrnd, value_type::list_flag, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		stack.push(value{}.set<value_type::list_flag>(
			_list_table.lrnd(vals[0].get<value_type::list_flag>())
		));
	}

	void operation<Command::LIST_INT, value_type::string, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		inkAssert(vals[0].type() == value_type::string, "list_flag construction needs the list name as string as first argument!");
		inkAssert(vals[1].type() == value_type::int32, "list_flag construction needs the flag numeric value as second argument!");
		list_flag entry = _list_table.get_list_id(vals[0].get<value_type::string>());
		entry.flag = vals[1].get<value_type::int32>() - 1;
		stack.push(value{}.set<value_type::list_flag>(entry));
	}

	int get_limit(const value& val) {
		if(val.type() == value_type::int32) {
			return val.get<value_type::int32>() - 1;
		} else {
			inkAssert(val.type() == value_type::list_flag, "flag value must be a integer or a list_flag");
			return val.get<value_type::list_flag>().flag;
		}
	}
	void operation<Command::LIST_RANGE, value_type::list, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		inkAssert(vals[0].type() == value_type::list, "Can't get range of non list type!");
		stack.push(value{}.set<value_type::list>(_list_table.range(
						vals[0].get<value_type::list>(),
						get_limit(vals[1]),
						get_limit(vals[2]))));
	}
}

