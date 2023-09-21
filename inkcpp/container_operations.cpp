#include "stack.h"
#include "value.h"
#include "operations.h"
#include "story_impl.h"
#include "globals_impl.h"
#include "runner.h"

#include <iostream>

namespace ink::runtime::internal {

	void operation<Command::READ_COUNT_VAR, value_type::divert, void>::operator()(
			basic_eval_stack& stack, value* vals)
	{
		container_t id;
		bool success = _story.get_container_id(
			_story.instructions() + vals[0].get<value_type::divert>(),
			id);
		inkAssert(success, "failed to find container to read visit count!");
		stack.push(value{}.set<value_type::int32>(
				static_cast<int32_t>(_visit_counts.visits( id )
			)));
	}

	void operation<Command::TURNS, value_type::divert, void>::operator()(
		basic_eval_stack& stack, value* vals)
	{
		container_t id;
		bool success = _story.get_container_id(
			_story.instructions() + vals[0].get<value_type::divert>(),
			id);
		inkAssert(success, "failed to find container to read turn count!");
		stack.push(value{}.set<value_type::int32>(
						static_cast<int32_t>(_visit_counts.turns(id)
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
