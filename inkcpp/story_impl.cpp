#include "story_impl.h"
#include "platform.h"
#include "runner_impl.h"
#include "globals_impl.h"
#include "version.h"

#ifdef INK_ENABLE_STL
#include <iostream>
#endif

namespace ink::runtime
{

#ifdef INK_ENABLE_STL
	unsigned char* read_file_into_memory(const char* filename, size_t* read)
	{
		using namespace std;

		ifstream ifs(filename, ios::binary | ios::ate);
		ifstream::pos_type pos = ifs.tellg();
		size_t length = (size_t)pos;
		unsigned char* data = new unsigned char[length];
		ifs.seekg(0, ios::beg);
		ifs.read((char*)data, length);
		ifs.close();

		*read = (size_t)length;
		return data;
	}

	story::input::input(const char* filename) {
		_data = read_file_into_memory(filename, &_length);
		_freeOnDestroy = true;
	}
#endif

	story* story::create(input&& inkbin, input&& text)
	{
		return new internal::story_impl(std::move(inkbin), std::move(text));
	}
}

namespace ink::runtime::internal
{
	story_impl::story_impl(story::input&& inkbin, story::input&& text)
		: _inkbin{std::move(inkbin)}, _text_file{std::move(text)}
	{
		// Setup data section pointers
		setup_pointers();

		// create story block
		_block = new internal::ref_block();
		_block->references = 1;
	}

	story_impl::~story_impl()
	{
		// clear pointers
		_instruction_data = nullptr;
		_string_table = nullptr;

		// clear out our reference block
		_block->valid = false;
		internal::ref_block::remove_reference(_block);
		_block = nullptr;
	}

	const char* story_impl::string(uint32_t index) const
	{
		const char* str = _string_table;
		for (int i = 0; i < index; ++i) {
			while(*str++);
			ink_assert(*str, "index out of bounds!");
		}
		return str;
	}

	bool story_impl::iterate_containers(const uint32_t*& iterator, container_t& index, ip_t& offset, bool reverse) const
	{
		if (iterator == nullptr)
		{
			// Empty check
			if (_container_list_size == 0)
			{
				return false;
			}

			// Start
			iterator = reverse
				? _container_list + (_container_list_size - 1) * 2
				: _container_list;
		}
		else
		{
			// Range check
			inkAssert(iterator >= _container_list && iterator <= _container_list + _container_list_size * 2, "Container fail");

			// Advance
			iterator += reverse ? -2 : 2;

			// End?
			if (iterator >= _container_list + _container_list_size * 2 || iterator < _container_list)
			{
				iterator = nullptr;
				index = 0;
				offset = nullptr;
				return false;
			}
		}

		// Get metadata
		index = *(iterator + 1);
		offset = *iterator + instructions();
		return true;
	}

	bool story_impl::get_container_id(ip_t offset, container_t& container_id) const
	{
		const uint32_t* iter = nullptr;
		ip_t iter_offset = nullptr;
		while (iterate_containers(iter, container_id, iter_offset))
		{
			if (iter_offset == offset)
				return true;
		}

		return false;
	}

	ip_t story_impl::find_offset_for(hash_t path) const
	{
		hash_t* iter = _container_hash_start;

		while (iter != _container_hash_end)
		{
			if (*iter == path)
			{
				return instructions() + *(offset_t*)(iter + 1);
			}

			iter += 2;
		}

		return nullptr;
	}

	globals story_impl::new_globals()
	{
		// create the new globals store
		return globals(new globals_impl(this), _block);
	}

	runner story_impl::new_runner(globals store)
	{
		if (store == nullptr)
			store = new_globals();
		return runner(new runner_impl(this, store), _block);
	}

	void story_impl::setup_pointers()
	{
		// String table is after the header
		_string_table = (char*)_text_file.data();


		char* ptr = reinterpret_cast<char*>(_inkbin.data());
		using header = ink::internal::header;
		_header = header::parse_header(ptr);
		ptr += header::Size;

		_num_containers = *reinterpret_cast<uint32_t*>(ptr);
		ptr += sizeof(uint32_t);

		// Pass over the container data
		_container_list_size = 0;
		_container_list = reinterpret_cast<uint32_t*>(ptr);
		while (true)
		{
			uint32_t val = *reinterpret_cast<uint32_t*>(ptr);
			if (val == ~0)
			{
				ptr += sizeof(uint32_t);
				break;
			}
			else
			{
				ptr += sizeof(uint32_t) * 2;
				_container_list_size++;
			}
		}

		// Next is the container hash map
		_container_hash_start = (hash_t*)(ptr);
		while (true)
		{
			uint32_t val = *(uint32_t*)ptr;
			if (val == ~0)
			{
				_container_hash_end = (hash_t*)(ptr);
				ptr += sizeof(uint32_t);
				break;
			}

			ptr += sizeof(uint32_t) * 2;
		}

		// After strings comes instruction data
		_instruction_data = (ip_t)ptr;

		// Debugging info
		/*{
			const uint32_t* iter = nullptr;
			container_t index; ip_t offset;
			while (this->iterate_containers(iter, index, offset))
			{
				std::clog << "Container #" << index << ": " << (int)offset << std::endl;
			}
		}*/
	}
}
