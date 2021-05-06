/// implements operations on lists

#include "stack.h"
#include "value.h"
#include "operations.h"
#include "list_table.h"

#define call4(RET0, RET1, RET2, RET3, FUN) \
	if (vals[0].type == value_type::list_flag) { \
		if(vals[1].type == value_type::list) { \
			stack.push(value{}.set<value_type::RET0>( \
				_list_table.FUN( \
					vals[0].get<value_type::list_flag>(), \
					vals[1].get<value_type::list>() \
				) \
			)); \
		} else {\
			inkAssert(vals[1].type()==value_type::list_flag);\
			stack.push(value{}.set<value_type::RET1>( \
					_list_table.FUN( \
						vals[0].get<value_type::list_flag>(), \
						vals[1].get<value_type::list_flag>() \
					)\
			)); \
		} \
	} else { \
		inkAssert(vals[0].type() == value_type::list); \
		if(vals[1].type == value_type::list) { \
			stack.push(value{}.set<value_type::RET2>( \
				_list_table.FUN( \
					vals[0].get<value_type::list>(), \
					vals[1].get<value_type::list>() \
				) \
			)); \
		} else {\
			inkAssert(vals[1].type()==value_type::list_flag);\
			stack.push(value{}.set<value_type::RET3>( \
					_list_table.FUN( \
						vals[0].get<value_type::list>(), \
						vals[1].get<value_type::list_flag>() \
					)\
			)); \
		} \
	}

#define call2(RET0, RET1, FUN) \
	if (vals[0].type() == value_type::list_flag) { \
		stack.push(value{}.set<value_type::RET0>( \
			_list_table.FUN(vals[0].get<value_type::list_flag>()) \
		)); \
	} else { \
		inkAssert(vals[0].type == value_type::list); \
		stack.push(value{}.set<value_type::RET1>( \
			_list_table.FUN(vals[0].get<value_type::list_flag>()) \
		)); \
	} \

namespace ink::runtime::internal {

}

