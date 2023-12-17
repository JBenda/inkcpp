#include "story_ptr.h"

namespace ink::runtime::internal
{
	void ref_block::remove_reference(ref_block*& block)
	{
		if (block == nullptr)
			return;

		// If we only have one references left
		if (block->references <= 1)
		{
			// delete the block
			delete block;
			block = nullptr;
			return;
		}

		// Otherwise, derecement references
		block->references--;
	}

	story_ptr_base::story_ptr_base(internal::ref_block* story)
		: _story_block(story)
	{
		_instance_block = new ref_block();
	}

	story_ptr_base::story_ptr_base(internal::ref_block* story, internal::ref_block* instance)
		: _story_block(story), _instance_block(instance)
	{
	}

	story_ptr_base::story_ptr_base(const story_ptr_base& other)
		: _story_block(other._story_block)
		, _instance_block(other._instance_block)
	{
	}

	void story_ptr_base::set(const story_ptr_base& other)
	{
		_story_block = other._story_block;
		_instance_block = other._instance_block;
	}

	void story_ptr_base::add_reference()
	{
		// If our block isn't valid, don't bother
		if (_story_block == nullptr || _instance_block == nullptr || !_story_block->valid || !_instance_block->valid)
		{
			_story_block = _instance_block = nullptr;
			return;
		}

		_instance_block->references++;
		_story_block->references++;
	}

	bool story_ptr_base::remove_reference()
	{
		ref_block::remove_reference(_story_block);
		ref_block::remove_reference(_instance_block);

		bool is_destroyed = _instance_block == nullptr;

		_instance_block = _story_block = nullptr;
		return is_destroyed;
	}
  } // namespace ink::runtime::internal
