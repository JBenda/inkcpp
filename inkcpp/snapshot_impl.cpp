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

	void snapshot::write_to_file(const char* filename) const
	{
		std::ofstream ofs(filename, std::ios::binary);
		if(!ofs.is_open()) {
			throw ink_exception("Failed to open file to write snapshot: "
				+ std::string(filename));
		}
		ofs.write(reinterpret_cast<const char*>(get_data()), get_data_len());
	}
#endif
}

namespace ink::runtime::internal
{
	size_t snapshot_impl::file_size(size_t serialization_length, size_t runner_cnt) {
		return serialization_length + sizeof(header) + (runner_cnt + 1) * sizeof(size_t);
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
			globals.strings(),
			globals._owner->string(0)
		};
		_length = globals.snap(nullptr, snapper);
		size_t runner_cnt = 0;
		for(auto node = globals._runners_start; node; node = node->next)
		{
			_length += node->object->snap(nullptr, snapper);
			++runner_cnt;
		}
		
		_length = file_size(_length, runner_cnt);
		_header.length = _length;
		_header.num_runners = runner_cnt;
		unsigned char* data = new unsigned char[_length];
		_file = data;
		unsigned char* ptr = data;
		// write header
		memcpy(ptr, &_header, sizeof(_header));
		// write lookup table
		ptr += sizeof(header);
		{
			size_t offset = (ptr - data) + (_header.num_runners + 1) * sizeof(size_t);
			memcpy(ptr, &offset, sizeof(offset));
			ptr += sizeof(offset);
			offset += globals.snap(nullptr, snapper);
			for(auto node = globals._runners_start; node; node = node->next)
			{
				memcpy(ptr, &offset, sizeof(offset));
				ptr += sizeof(offset);
				offset += node->object->snap(nullptr, snapper);
			}
		}

		ptr += globals.snap(ptr, snapper);
		for (auto node = globals._runners_start; node; node = node->next)
		{
			ptr += node->object->snap(ptr, snapper);
		}
	}

	snapshot_impl::snapshot_impl(const unsigned char* data, size_t length, bool managed)
	: _file{data}, _length{length}, _managed{managed}
	{
		const unsigned char* ptr = data;
		memcpy(&_header, ptr, sizeof(_header));
		inkAssert(_header.length == _length, "Corrupted file length");
	}

	
	size_t snap_choice::snap(unsigned char* data, const snapper& snapper) const{
		unsigned char* ptr = data;
		bool should_write = data != nullptr;
		ptr = snap_write(ptr, _index, should_write);
		ptr = snap_write(ptr, _path, should_write);
		ptr = snap_write(ptr, _thread, should_write);
		// handle difference between no tag and first tag
		if (_tags == nullptr) {
			ptr = snap_write(ptr, false, should_write);
		} else {
			ptr = snap_write(ptr, true, should_write);
			std::uintptr_t offset = _tags != nullptr ? _tags - snapper.current_runner_tags : 0;
			ptr = snap_write(ptr, offset, should_write);
		}
		ptr = snap_write(ptr, snapper.strings.get_id(_text), should_write);
		return ptr - data;
	}

	const unsigned char* snap_choice::snap_load(const unsigned char* data, const loader& loader){
		const unsigned char* ptr = data;
		ptr = snap_read(ptr, _index);
		ptr = snap_read(ptr, _path);
		ptr = snap_read(ptr, _thread);
		bool has_tags;
		ptr = snap_read(ptr, has_tags);
		if (has_tags) {
			std::uintptr_t offset;
			ptr = snap_read(ptr, offset);
			_tags = loader.current_runner_tags + offset;
		} else {
			_tags = nullptr;
		}
		size_t string_id;
		ptr = snap_read(ptr, string_id);
		_text = loader.string_table[string_id];
		return ptr;
	}
	size_t snap_tag::snap(unsigned char* data, const snapper& snapper) const{
		unsigned char* ptr = data;
		bool should_write = data != nullptr;
		if (_str == nullptr) {
			ptr = snap_write(ptr, false, should_write);
		} else {
			size_t id = snapper.strings.get_id(_str);
			ptr = snap_write(ptr, true, should_write);
			ptr = snap_write(ptr, id, should_write);
		}
		return ptr - data;
	}
	const unsigned char* snap_tag::snap_load(const unsigned char* data, const loader& loader){
		const unsigned char* ptr = data;
		bool has_content;
		ptr = snap_read(ptr, has_content);
		if (!has_content) {
			_str = nullptr;
		} else {
			size_t id;
			ptr = snap_read(ptr, id);
			_str = loader.string_table[id];
		}
		return ptr;
	}
}
