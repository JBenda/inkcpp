/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "story_impl.h"
#include "platform.h"
#include "runner_impl.h"
#include "globals_impl.h"
#include "snapshot.h"
#include "snapshot_impl.h"
#include "snapshot_interface.h"
#include "version.h"

#include "..\..\..\sil\sil\main.h"
#include "..\..\..\sil\sil\profile.h"


namespace ink::runtime
{
#ifdef INK_ENABLE_STL
story* story::from_file(const char* filename) { return new internal::story_impl(filename); }
#endif

story* story::from_binary(unsigned char* data, size_t length, bool freeOnDestroy)
{
	return new internal::story_impl(data, length, freeOnDestroy);
}
} // namespace ink::runtime

namespace ink::runtime::internal
{

#ifdef INK_ENABLE_STL
unsigned char* read_file_into_memory(const char* filename, size_t* read)
{
	using namespace std;

	ifstream ifs(filename, ios::binary | ios::ate);

	if (! ifs.is_open()) {
		throw ink_exception("Failed to open file: " + std::string(filename));
	}

	ifstream::pos_type pos    = ifs.tellg();
	size_t             length = ( size_t ) pos;
	unsigned char*     data   = new unsigned char[length];
	ifs.seekg(0, ios::beg);
	ifs.read(( char* ) data, length);
	ifs.close();

	*read = ( size_t ) length;
	return data;
}

story_impl::story_impl(const char* filename)
    : _file(nullptr)
    , _length(0)
    , _string_table(nullptr)
    , _instruction_data(nullptr)
    , _managed(true)
{
	// Load file into memory
	_file = read_file_into_memory(filename, &_length);

	// Find all the right data sections
	setup_pointers();

	// create story block
	_block             = new internal::ref_block();
	_block->references = 1;
}
#endif

story_impl::story_impl(unsigned char* binary, size_t len, bool manage /*= true*/)
    : _file(binary)
    , _length(len)
    , _managed(manage)
{
	// Setup data section pointers
	setup_pointers();

	// create story block
	_block             = new internal::ref_block();
	_block->references = 1;
}

story_impl::~story_impl()
{
	// delete file memory if we're responsible for it
	if (_file != nullptr && _managed)
		delete[] _file;

	// clear pointers
	_file             = nullptr;
	_instruction_data = nullptr;
	_string_table     = nullptr;

	// clear out our reference block
	_block->valid = false;
	internal::ref_block::remove_reference(_block);
	_block = nullptr;
}

const char* story_impl::string(uint32_t index) const { return _string_table + index; }


bool story_impl::find_container_id(uint32_t offset, container_t& container_id) const
{
	// Find inmost container.
	container_id = find_container_for(offset);

	// Exact match?
	return container(container_id)._start_offset == offset;
}

// Search sorted looking for the target or the largest value smaller than target.
// Assumes 2*count u32s, with the sorted values in the even slots. The odd slots can have any payload.
static const uint32_t *upper_bound(const uint32_t *sorted, uint32_t count, uint32_t target)
{
	if (count == 0)
		return nullptr;

	uint32_t begin	= 0;
	uint32_t end	= count;

	while (begin < end)	{
		const uint32_t mid = begin + (end - begin + 1) / 2;
		const uint32_t mid_offset = sorted[mid * 2 + 0];

		if (mid_offset > target)
			// Look below
			end = mid - 1;
		else
			// Look above
			begin = mid;
	}

	return sorted + begin * 2;
}

container_t story_impl::find_container_for(uint32_t offset) const
{
	// Container map contains offsets in even slots, container ids in odd.
	const uint32_t *iter = upper_bound(_container_list, _container_list_size, offset);

	// The last container command before the offset could be either the start of a container
	// (in which case the offset is contained within) or the end of a container, in which case
	// the offset is inside that container's parent.

	// If we're not inside the container, walk out to find the actual parent. Normally we'd 
	// know that the parent contained the child, but the containers are sparse so we might 
	// not have anything.
	container_t id = iter ? iter[1] : ~0;
	while (id != ~0)
	{
		const Container& c = container(id);
		if (c._start_offset <= offset && c._end_offset >= offset)
			return id;

		id = c._parent;
	}

	return id;
}

CommandFlag story_impl::container_flag(ip_t offset) const
{
	inkAssert(
	    (static_cast<Command>(offset[0]) == Command::START_CONTAINER_MARKER
	     || static_cast<Command>(offset[0]) == Command::END_CONTAINER_MARKER),
	    "Tried to fetch container flag from non container command!"
	);
	return static_cast<CommandFlag>(offset[1]);
}

ip_t story_impl::find_offset_for(hash_t path) const
{	
	// Hash map contains hashes in even slots, offsets in odd.
	const uint32_t count = (_container_hash_end - _container_hash_start) / 2;
	const hash_t *iter = upper_bound(_container_hash_start, count, path);

	return iter[0] == path ? _instruction_data + iter[1] : nullptr;
}

globals story_impl::new_globals()
{
	// create the new globals store
	return globals(new globals_impl(this), _block);
}

globals story_impl::new_globals_from_snapshot(const snapshot& data)
{
	const snapshot_impl& snapshot = reinterpret_cast<const snapshot_impl&>(data);
	auto*                globs    = new globals_impl(this);
	auto                 end      = globs->snap_load(
      snapshot.get_globals_snap(),
      snapshot_interface::loader{
          snapshot.strings(),
          _string_table,
      }
  );
	inkAssert(end == snapshot.get_runner_snap(0), "not all data were used for global reconstruction");
	return globals(globs, _block);
}

runner story_impl::new_runner(globals store)
{
	if (store == nullptr)
		store = new_globals();
	return runner(new runner_impl(this, store), _block);
}

runner story_impl::new_runner_from_snapshot(const snapshot& data, globals store, unsigned idx)
{
	const snapshot_impl& snapshot = reinterpret_cast<const snapshot_impl&>(data);
	if (store == nullptr)
		store = new_globals_from_snapshot(snapshot);
	auto* run    = new runner_impl(this, store);
	auto  loader = snapshot_interface::loader{
      snapshot.strings(),
      _string_table,
  };
	// snapshot id is inverso of creation time, but creation time is the more intouitve numbering to
	// use
	idx      = (data.num_runners() - idx - 1);
	auto end = run->snap_load(snapshot.get_runner_snap(idx), loader);
	inkAssert(
	    (idx + 1 < snapshot.num_runners() && end == snapshot.get_runner_snap(idx + 1))
	        || end == snapshot.get_data() + snapshot.get_data_len(),
	    "not all data were used for runner reconstruction"
	);
	return runner(run, _block);
}

void story_impl::setup_pointers()
{
	using header = ink::internal::header;
	_header      = header::parse_header(reinterpret_cast<char*>(_file));

	// String table is after the header
	_string_table = ( char* ) _file + header::Size;

	// Pass over strings
	const char* ptr = _string_table;
	if (*ptr == 0) // SPECIAL: No strings
	{
		ptr++;
	} else
		while (true) {
			// Read until null terminator
			while (*ptr != 0)
				ptr++;

			// Check next character
			ptr++;

			// Second null. Strings are done.
			if (*ptr == 0) {
				ptr++;
				break;
			}
		}

	// check if lists are defined
	_list_meta = ptr;
	if (list_flag flag = _header.read_list_flag(ptr); flag != null_flag) {
		// skip list definitions
		auto list_id = flag.list_id;
		while (*ptr != 0) {
			++ptr;
		}
		++ptr; // skip list name
		do {
			if (flag.list_id != list_id) {
				list_id = flag.list_id;
				while (*ptr != 0) {
					++ptr;
				}
				++ptr; // skip list name
			}
			while (*ptr != 0) {
				++ptr;
			}
			++ptr; // skip flag name
		} while ((flag = _header.read_list_flag(ptr)) != null_flag);

		_lists = reinterpret_cast<const list_flag*>(ptr);
		// skip predefined lists
		while (_header.read_list_flag(ptr) != null_flag) {
			while (_header.read_list_flag(ptr) != null_flag)
				;
		}
	} else {
		_list_meta = nullptr;
		_lists     = nullptr;
	}
	inkAssert(
	    _header.ink_bin_version_number == ink::InkBinVersion,
	    "invalid InkBinVerison! currently: %i you used %i", ink::InkBinVersion,
	    _header.ink_bin_version_number
	);
	inkAssert(
	    _header.endien == header::endian_types::same, "different endien support not yet implemented"
	);


	_num_containers = *( uint32_t* ) (ptr);
	ptr += sizeof(uint32_t);

	// Pass over the container data
	_container_list_size = 0;
	_container_list      = ( uint32_t* ) (ptr);
	while (true) {
		uint32_t val = *( uint32_t* ) ptr;
		if (val == ~0) {
			ptr += sizeof(uint32_t);
			break;
		} else {
			ptr += sizeof(uint32_t) * 2;
			_container_list_size++;
		}
	}

	// Next is the container hash map
	_container_hash_start = ( hash_t* ) (ptr);
	while (true) {
		uint32_t val = *( uint32_t* ) ptr;
		if (val == ~0) {
			_container_hash_end = ( hash_t* ) (ptr);
			ptr += sizeof(uint32_t);
			break;
		}

		ptr += sizeof(uint32_t) * 2;
	}

	// After strings comes instruction data
	_instruction_data = ( ip_t ) ptr;

	if (_num_containers) {
		container_t *stack = new container_t[_num_containers];
		uint32_t depth = 0;
		stack[depth] = ~0;

		// Build acceleration structure for containers.
		_containers = new Container[_num_containers];
		for (uint32_t c = 0; c < _container_list_size; ++c)
		{
			const uint32_t *iter = _container_list + 2 * c;
			const container_t id = iter[1];
			const uint32_t offset = iter[0];

			const Command command = Command(_instruction_data[offset]);

			inkAssert(command == Command::START_CONTAINER_MARKER || command == Command::END_CONTAINER_MARKER);

			if (command == Command::START_CONTAINER_MARKER)
			{
				_containers[id]._start_offset = offset;
				_containers[id]._flags = CommandFlag(_instruction_data[offset+1]);
				_containers[id]._parent = stack[depth];

				inkAssert(_containers[id]._flags != CommandFlag(0));

				stack[++depth] = id;
			}
			else
			{
				_containers[stack[depth]]._end_offset = offset;
				--depth;
			}

			for (uint32_t *h = _container_hash_start; h < _container_hash_end; h += 2)
			{
				if (h[1] == offset)
				{
					_containers[id]._hash = h[0];
					break;
				}
			}
		}

		delete[] stack;
	}

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
} // namespace ink::runtime::internal
