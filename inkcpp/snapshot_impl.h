#pragma once

#include "snapshot.h"

#include <cstring>

namespace ink::runtime::internal
{
	class globals_impl;
	class value;
	class string_table;
	class snapshot_interface {
	protected:
		static unsigned char* snap_write(unsigned char* ptr, const void* data, size_t length, bool write)
		{
			memcpy(ptr, data, length);
			return ptr += length;
		}
		template<typename T>
		static unsigned char* snap_write(unsigned char* ptr, const T& data, bool write)
		{
				return snap_write(ptr, &data, sizeof(data), write);
		}
	public:
		struct snapper {
			const string_table& strings;
		};
		struct loader {
			string_table& strings;
		};
		virtual size_t snap(unsigned char* data, const snapper&) const = 0;
		virtual const unsigned char* snap_load(const unsigned char* data, const loader&) = 0;
	};
	class snapshot_impl : public snapshot
	{
	public:
		~snapshot_impl() override {
			if (_managed) { delete[] _file; }
		};
		const unsigned char* get_data() const override;
		size_t get_data_len() const override;

		snapshot_impl(const globals_impl&);
			// write down all allocated strings
			// replace pointer with idx
			// reconsrtuct static strings index
			// list_table _data & _entry_state
		snapshot_impl(const unsigned char* data, size_t length, bool managed);

	private:
		// file information
		const unsigned char* _file;
		size_t _length;
		bool _managed;
		static size_t file_size(size_t, size_t);
		struct header {
			size_t num_runners;
			size_t length;

		} _header;
	};
}
