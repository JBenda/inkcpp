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
		ink_assert(false, "Failed to open file: %s", filename);
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
	return container_data(container_id)._start_offset == offset;
}

// Search sorted looking for the target or the largest value smaller than target.
template<typename entry>
static const entry* upper_bound(const entry* sorted, uint32_t count, uint32_t key)
{
	if (count == 0)
		return nullptr;

	uint32_t begin = 0;
	uint32_t end   = count;

	while (begin < end) {
		const uint32_t mid     = begin + (end - begin + 1) / 2;
		const uint32_t mid_key = sorted[mid].key();

		if (mid_key > key)
			// Look below
			end = mid - 1;
		else
			// Look above
			begin = mid;
	}

	return sorted + begin;
}

container_t story_impl::find_container_for(uint32_t offset) const
{
	// Container map contains offsets in even slots, container ids in odd.
	const container_map_t* entry = upper_bound(_container_map, _container_map_size, offset);

	// The last container command before the offset could be either the start of a container
	// (in which case the offset is contained within) or the end of a container, in which case
	// the offset is inside that container's parent.

	// If we're not inside the container, walk out to find the actual parent. Normally we'd
	// know that the parent contained the child, but the containers are sparse so we might
	// not have anything.
	container_t id = entry ? entry->_id : ~0;
	while (id != ~0) {
		const container_data_t& data = container_data(id);
		if (data._start_offset <= offset && data._end_offset >= offset)
			return id;

		id = data._parent;
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
	const container_hash_t* entry = upper_bound(_container_hash, _container_hash_size, path);

	return entry && entry->_hash == path ? _instruction_data + entry->_offset : nullptr;
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
	const ink::internal::header& header = *reinterpret_cast<const ink::internal::header*>(_file);
	if (! header.verify())
		return;

	// Locate sections
	if (header._strings._bytes)
		_string_table = reinterpret_cast<char*>(_file + header._strings._start);

	// Address list sections if they exist
	if (header._list_meta._bytes) {
		_list_meta = reinterpret_cast<const char*>(_file + header._list_meta._start);

		// Lists require metadata
		if (header._lists._bytes)
			_lists = reinterpret_cast<const list_flag*>(_file + header._lists._start);
	}

	// Address containers section if it exists
	if (header._containers._bytes) {
		_num_containers = header._containers._bytes / sizeof(container_data_t);
		_container_data = reinterpret_cast<const container_data_t*>(_file + header._containers._start);
	}

	// Address container map if it exists
	if (header._container_map._bytes) {
		_container_map_size = header._container_map._bytes / sizeof(container_map_t);
		_container_map = reinterpret_cast<const container_map_t*>(_file + header._container_map._start);
	}

	// Address container hash if it exists
	if (header._container_hash._bytes) {
		_container_hash_size = header._container_hash._bytes / sizeof(container_hash_t);
		_container_hash
		    = reinterpret_cast<const container_hash_t*>(_file + header._container_hash._start);
	}

	// Address instructions, which we hope exist!
	if (header._instructions._bytes)
		_instruction_data = _file + header._instructions._start;

	// Shrink file length to fit exact length of instructions section.
	inkAssert(end() >= _instruction_data + header._instructions._bytes);
	_length = _instruction_data + header._instructions._bytes - _file;

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
