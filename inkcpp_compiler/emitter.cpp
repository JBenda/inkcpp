#include "emitter.h"

namespace ink::compiler::internal
{
	void emitter::start(int ink_version, compilation_results* results)
	{
		// store
		_ink_version = ink_version;
		set_results(results);

		// reset
		_container_map.clear();
		_max_container_index = 0;

		// initialize
		initialize();
	}

	void emitter::finish(container_t max_container_index)
	{
		// store max index
		_max_container_index = max_container_index;

		// finalize
		finalize();
	}

	void emitter::add_start_to_container_map(uint32_t offset, container_t index)
	{
		if (_container_map.rbegin() != _container_map.rend())
		{
			if (_container_map.rbegin()->first > offset)
			{
				warn() << "Container map written out of order. Wrote container at offset "
					<< offset << " after container with offset " << _container_map.rbegin()->first << std::flush;
			}
		}

		_container_map.push_back(std::make_pair(offset, index));
		setContainerIndex(index);
	}

	void emitter::add_end_to_container_map(uint32_t offset, container_t index)
	{
		if (_container_map.rbegin() != _container_map.rend())
		{
			if (_container_map.rbegin()->first > offset)
			{
				warn() << "Container map written out of order. Wrote container at offset "
					<< offset << " after container with offset " << _container_map.rbegin()->first << std::flush;
			}
		}

		_container_map.push_back(std::make_pair(offset, index));
	}
}
