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
	story* story::from_file(const char* filename)
	{
		return new internal::story_impl(filename);
	}
#endif

	story* story::from_binary(unsigned char* data, size_t length, bool freeOnDestroy)
	{
		return new internal::story_impl(data, length, freeOnDestroy);
	}
}

namespace ink::runtime::internal
{

#ifdef INK_ENABLE_STL
	unsigned char* read_file_into_memory(const char* filename, size_t* read)
	{
		using namespace std;

		ifstream ifs(filename, ios::binary | ios::ate);

		if (!ifs.is_open()) {
			throw ink_exception("Failed to open file: " + std::string(filename));
		}

		ifstream::pos_type pos = ifs.tellg();
		size_t length = (size_t)pos;
		unsigned char* data = new unsigned char[length];
		ifs.seekg(0, ios::beg);
		ifs.read((char*)data, length);
		ifs.close();

		*read = (size_t)length;
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
		_block = new internal::ref_block();
		_block->references = 1;
	}
#endif

	story_impl::story_impl(unsigned char* binary, size_t len, bool manage /*= true*/)
		: _file(binary), _length(len), _managed(manage)
	{
		// Setup data section pointers
		setup_pointers();

		// create story block
		_block = new internal::ref_block();
		_block->references = 1;
	}

	story_impl::~story_impl()
	{
		// delete file memory if we're responsible for it
		if (_file != nullptr && _managed)
			delete[] _file;

		// clear pointers
		_file = nullptr;
		_instruction_data = nullptr;
		_string_table = nullptr;

		// clear out our reference block
		_block->valid = false;
		internal::ref_block::remove_reference(_block);
		_block = nullptr;
	}

	const char* story_impl::string(uint32_t index) const
	{
		return _string_table + index;
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

	
	CommandFlag story_impl::container_flag(ip_t offset) const {
		inkAssert((static_cast<Command>(offset[0]) == Command::START_CONTAINER_MARKER ||
			static_cast<Command>(offset[0]) == Command::END_CONTAINER_MARKER), "Tried to fetch container flag from non container command!");
		return static_cast<CommandFlag>(offset[1]);
	}
	CommandFlag story_impl::container_flag(container_t id) const {
		const uint32_t* iter = nullptr;
		ip_t offset;
		container_t c_id;
		while(iterate_containers(iter, c_id, offset)) {
			if (c_id == id) {
				inkAssert(static_cast<Command>(offset[0]) == Command::START_CONTAINER_MARKER, "Container list pointer is invalid!");
				return static_cast<CommandFlag>(offset[1]);
			}
		}
		inkFail("Container not found -> can't fetch flag");
		return CommandFlag::NO_FLAGS;
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

	globals story_impl::new_globals_from_snapshot(const snapshot& data)
	{
		const snapshot_impl& snapshot = reinterpret_cast<const snapshot_impl&>(data);
		auto* globs = new globals_impl(this);
		auto end = globs->snap_load(snapshot.get_globals_snap(), snapshot_interface::loader{
			snapshot.strings(),
			_string_table,
		});
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
		auto* run = new runner_impl(this, store);
		auto loader = snapshot_interface::loader{
					snapshot.strings(),
					_string_table, 
		};
		auto end = run->snap_load(snapshot.get_runner_snap(idx), loader);
		inkAssert(
			(idx + 1 < snapshot.num_runners() && end == snapshot.get_runner_snap(idx + 1))
				|| end == snapshot.get_data() + snapshot.get_data_len(), "not all data were used for runner reconstruction"
			);
		return runner(run, _block);
	}

	void story_impl::setup_pointers()
	{
		using header = ink::internal::header;
		_header = header::parse_header(reinterpret_cast<char*>(_file));

		// String table is after the header
		_string_table = (char*)_file + header::Size;

		// Pass over strings
		const char* ptr = _string_table;
		if (*ptr == 0) // SPECIAL: No strings
		{
			ptr++;
		}
		else while (true)
		{
			// Read until null terminator
			while (*ptr != 0)
				ptr++;

			// Check next character
			ptr++;

			// Second null. Strings are done.
			if (*ptr == 0)
			{
				ptr++;
				break;
			}
		}

		// check if lists are defined
		_list_meta = ptr;
		if(list_flag flag = _header.read_list_flag(ptr); flag != null_flag) {
			// skip list definitions
			auto list_id = flag.list_id;
			while(*ptr != 0) {++ptr;} ++ptr; // skip list name
			do{
				if(flag.list_id != list_id) {
					list_id = flag.list_id;
					while(*ptr != 0) {++ptr;} ++ptr; // skip list name
				}
				while(*ptr != 0) { ++ptr; } ++ptr; // skip flag name
			} while  ((flag = _header.read_list_flag(ptr)) != null_flag);

			_lists = reinterpret_cast<const list_flag*>(ptr);
			// skip predefined lists
			while(_header.read_list_flag(ptr) != null_flag) {
				while(_header.read_list_flag(ptr) != null_flag);
			}
		} else {
			_list_meta = nullptr;
			_lists = nullptr;
		}
		inkAssert(_header.ink_bin_version_number == ink::InkBinVersion, "invalid InkBinVerison! currently: %i you used %i", ink::InkBinVersion, _header.ink_bin_version_number);
		inkAssert(_header.endien == header::endian_types::same,
				"different endien support not yet implemented");



		_num_containers = *(uint32_t*)(ptr);
		ptr += sizeof(uint32_t);

		// Pass over the container data
		_container_list_size = 0;
		_container_list = (uint32_t*)(ptr);
		while (true)
		{
			uint32_t val = *(uint32_t*)ptr;
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
