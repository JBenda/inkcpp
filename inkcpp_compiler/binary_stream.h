#pragma once

#include "system.h"
#include <string>
#include <vector>
#include <ostream>

namespace ink
{
	namespace compiler
	{
		namespace internal
		{
			// Simple stream for writing data to an expanding binary stream in memory
			class binary_stream
			{
			public:
				binary_stream();
				~binary_stream();

				// Writes an arbitrary type
				template<typename T>
				size_t write(const T& value)
				{
					return write((const byte_t*)&value, sizeof(T));
				}

				// Write a string plus a null terminator

				// Writes data to the end of the stream
				size_t write(const byte_t* data, size_t len);

				// Writes the data in the buffer into another stream
				void write_to(std::ostream& out) const;

				// Gets the current position in memory of the write head
				size_t pos() const;

				template<typename T>
				void set(size_t offset, const T& value)
				{
					set(offset, (const byte_t*)&value, sizeof(T));
				}

				// Writes data into an old position
				void set(size_t offset, const byte_t* data, size_t len);

				// reset to 0
				void reset();

			private:
				// Size of a data slab. Whenever
				//  a slab runs out of data,
				//  a new slab is allocated.
				const size_t DATA_SIZE = 256;

				// Prior slabs
				std::vector<byte_t*> _slabs;

				// Current slab being written to
				byte_t* _currentSlab = nullptr;

				// Write head
				byte_t* _ptr = nullptr;
			};
			template<>
			size_t binary_stream::write(const std::string& value);
		}
	}
}
