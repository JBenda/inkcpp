#include "stack.h"
#include "value.h"
#include "operations.h"
#include "story_impl.h"
#include "globals_impl.h"

#include <iostream>

namespace ink::runtime::internal {

	void operation<Command::READ_COUNT_VAR, value_type::divert, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		inkAssert(vals[0].type() == value_type::divert);
		ip_t offset
			= _story.instructions() + vals[0].get<value_type::divert>();
		const uint32_t* iter = nullptr;
		ip_t iter_offset = nullptr;
		container_t container_id;
		while(_story.iterate_containers(iter, container_id, iter_offset))
		{
			if(iter_offset == offset)
			{
				stack.push(value{}.set<value_type::int32>(
					static_cast<int32_t>(_visit_counts.visits(container_id))));
				return;
			}
		}
		inkAssert(0, "failed to find read count target!");
	}

}
