#include "binary_stream.h"

#include <cstring>

namespace ink
{
	namespace compiler
	{
		namespace internal
		{
			template<>
			size_t binary_stream::write(const std::string& value)
			{
				constexpr byte_t ZERO = 0;
				size_t len = write((const byte_t*)value.c_str(), value.length());
				len += write(&ZERO, 1);
				return len;
			}

			binary_stream::binary_stream()
				: _currentSlab(nullptr)
				, _ptr(nullptr)
			{ }

			binary_stream::~binary_stream()
			{
				reset();
			}

			size_t binary_stream::write(const byte_t* data, size_t len)
			{
				// Create first slab if none exist
				if (_currentSlab == nullptr)
				{
					_currentSlab = new byte_t[DATA_SIZE];
					_ptr = _currentSlab;
				}

				// Check how much space we have left
				size_t slab_remaining = _currentSlab + DATA_SIZE - _ptr;

				// If we're out of space...
				if (slab_remaining < len)
				{
					// Write what we can into the slab
					memcpy(_ptr, data, slab_remaining);

					// Create a new slab
					_slabs.push_back(_currentSlab);
					_currentSlab = new byte_t[DATA_SIZE];
					_ptr = _currentSlab;

					// Recurse
					return slab_remaining + write(data + slab_remaining, len - slab_remaining);
				}

				// We have the space
				memcpy(_ptr, data, len);
				_ptr += len;
				return len;
			}

			void binary_stream::write_to(std::ostream& out) const
			{
				// Write previous slabs
				for (byte_t* slab : _slabs)
				{
					out.write(reinterpret_cast<const char*>(slab), DATA_SIZE);
				}

				// Write current slab (if it exists)
				if (_currentSlab != nullptr && _ptr != _currentSlab)
				{
					out.write(reinterpret_cast<const char*>(_currentSlab), _ptr - _currentSlab);
				}
			}

			size_t binary_stream::pos() const
			{
				// If we have no data, we're at position 0
				if (_currentSlab == nullptr)
					return 0;

				// Each slabs is size DATA_SIZE then add the position in the current slab
				return _slabs.size() * DATA_SIZE + (_ptr - _currentSlab);
			}

			void binary_stream::set(size_t offset, const byte_t* data, size_t len)
			{
				// Find slab for offset
				unsigned int slab_index = offset / DATA_SIZE;
				size_t pos = offset % DATA_SIZE;

				// Get slab and ptr
				byte_t* slab = nullptr;
				if (slab_index < _slabs.size())
					slab = _slabs[slab_index];
				else if (slab_index == _slabs.size())
					slab = _currentSlab;

				if (slab == nullptr)
					return; // TODO: Error?

				byte_t* ptr = slab + pos;

				// Check if data will fit into slab
				if (pos + len > DATA_SIZE)
				{
					// Write only what we can fit
					size_t new_length = DATA_SIZE - pos;
					memcpy(ptr, data, new_length);

					// Recurse
					set(offset + new_length, data + new_length, len - new_length);
					return;
				}

				// Otherwise write the whole data
				memcpy(ptr, data, len);
			}

			void binary_stream::reset()
			{
				// Delete all slabs
				for (byte_t* slab : _slabs)
				{
					delete[] slab;
				}
				_slabs.clear();

				// Delete active slab (if it exists)
				if (_currentSlab != nullptr)
					delete[] _currentSlab;
				_currentSlab = _ptr = nullptr;
			}
		}
	}
}
