#include "snapshot_impl.h"

#include "story_impl.h"
#include "globals_impl.h"
#include "runner_impl.h"

#include <cstring>
#ifdef INK_ENABLE_STL
#include <fstream>
#endif

namespace ink::runtime
{
	snapshot* snapshot::from_binary(const unsigned char* data, size_t length, bool freeOnDestroy)
	{
		return new internal::snapshot_impl(data, length, freeOnDestroy);
	}

#ifdef INK_ENABLE_STL
	snapshot* snapshot::from_file(const char* filename) {
		std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
		if(!ifs.is_open()) {
			throw ink_exception("Failed to open snapshot file: " + std::string(filename));
		}

		size_t length = static_cast<size_t>(ifs.tellg());
		unsigned char* data = new unsigned char[length];
		ifs.seekg(0, std::ios::beg);
		ifs.read(reinterpret_cast<char*>(data), length);
		ifs.close();

		return from_binary(data, length);
	}
#endif
}

namespace ink::runtime::internal
{
	size_t snapshot_impl::file_size(size_t serialization_length, size_t runner_cnt) {
		return serialization_length + sizeof(header) + runner_cnt * sizeof(size_t);
	}

	const unsigned char* snapshot_impl::get_data() const {
		return _file;
	}
	size_t snapshot_impl::get_data_len() const {
		return _length;
	}

	snapshot_impl::snapshot_impl(const globals_impl& globals)
		: _managed{true}
	{
		snapshot_interface::snapper snapper{
			.strings = globals.strings(),
			.story_string_table = globals._owner->string(0)
		};
		_length = globals.snap(nullptr, snapper);
		size_t runner_cnt = 0;
		for(auto node = globals._runners_start; node; node = node->next)
		{
			_length += node->object->snap(nullptr, snapper);
			++runner_cnt;
		}
		
		_length = file_size(_length, runner_cnt);
		unsigned char* data = new unsigned char[_length];
		_file = data;
		unsigned char* ptr = data;
		// write header
		memcpy(ptr, &_header, sizeof(_header));
		// write lookup table
		ptr += sizeof(header);
		{
			size_t offset = 0;
			for(auto node = globals._runners_start; node; node = node->next)
			{
				offset += node->object->snap(nullptr, snapper);
				memcpy(ptr, &offset, sizeof(offset));
				ptr += sizeof(offset);
			}
		}

		ptr += globals.snap(ptr, snapper);
		for (auto node = globals._runners_start; node; node = node->next)
		{
			ptr += node->object->snap(ptr, snapper);
		}
	}
}
