#include "stack.h"
#include "value.h"
#include "operations.h"
#include "story_impl.h"
#include "globals_impl.h"
#include "runner.h"

#include <iostream>

namespace ink::runtime::internal {

	container_t containerAddressToId(const story_impl& story, uint32_t address) {
		ip_t offset
			= story.instructions() + address;
		const uint32_t* iter = nullptr;
		ip_t iter_offset = nullptr;
		container_t container_id;
		while(story.iterate_containers(iter, container_id, iter_offset))
		{
			if(iter_offset == offset)
			{
				return container_id;
			}
		}
		inkAssert(0, "failed to find read count target!");
		return ~0;
	}

	void operation<Command::READ_COUNT_VAR, value_type::divert, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		stack.push(value{}.set<value_type::int32>(
			static_cast<int32_t>(_visit_counts.visits(
					containerAddressToId(_story, vals[0].get<value_type::divert>())
					))));
	}

	void operation<Command::TURNS, value_type::divert, void>::operator()(
		basic_eval_stack& stack, value* vals)
	{
		stack.push(value{}.set<value_type::int32>(
						static_cast<int32_t>(_visit_counts.turns(containerAddressToId(
								_story,
								vals[0].get<value_type::divert>()
								))
					)));
	}

	void operation<Command::CHOICE_COUNT, value_type::none, void>::operator()
		(basic_eval_stack& stack, value* vals)
	{
		stack.push(value{}.set<value_type::int32>(static_cast<int32_t>(
						_runner.num_choices()
					)));
	}

}
