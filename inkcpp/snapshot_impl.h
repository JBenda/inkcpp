#pragma once

#include "snapshot.h"
#include "snapshot_interface.h"
#include "array.h"


namespace ink::runtime::internal
{

	class snapshot_impl : public snapshot
	{
	public:
		~snapshot_impl() override {
			if (_managed) { delete[] _file; }
		};

		managed_array<const char*, true, 5>& strings() const {
			return string_table;
		}
		const unsigned char* get_data() const override;
		size_t get_data_len() const override;

		snapshot_impl(const globals_impl&);
			// write down all allocated strings
			// replace pointer with idx
			// reconsrtuct static strings index
			// list_table _data & _entry_state
		snapshot_impl(const unsigned char* data, size_t length, bool managed);

		const unsigned char* get_globals_snap() const {
			return _file + get_offset(0);
		}

		const unsigned char* get_runner_snap(size_t idx) const {
			return _file + get_offset(idx + 1);
		}

		size_t num_runners() const override { return _header.num_runners; }

	private:
		// file information
		// only populated when loading snapshots
		mutable managed_array<const char*, true, 5> string_table;
		const unsigned char* _file;
		size_t _length;
		bool _managed;
		static size_t file_size(size_t, size_t);
		struct header {
			size_t num_runners;
			size_t length;

		} _header;

		size_t get_offset(size_t idx) const {
			inkAssert(idx <= _header.num_runners);
			return reinterpret_cast<const size_t*>(_file + sizeof(header))[idx];
		}

	};
}
