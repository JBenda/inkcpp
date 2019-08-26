#pragma once

namespace binary
{
	// Simple stream for writing data to an expanding binary stream
	class binary_stream
	{
	public:
		// Byte type
		typedef unsigned char byte;

		binary_stream();
		~binary_stream();

		// Writes an arbitrary type
		template<typename T>
		size_t write(const T& value)
		{
			return write((const byte*)&value, sizeof(T));
		}

		// Write a string plus a null terminator
		template<>
		size_t write(const std::string& value)
		{
			const byte ZERO = 0;
			size_t len = write((const byte*)value.c_str(), value.length());
			len += write(&ZERO, 1);
			return len;
		}

		// Writes data to the end of the stream
		size_t write(const byte* data, size_t len);

		// Writes the data in the buffer into another stream
		void write_to(std::ostream& out) const;

		// Gets the current position in memory of the write head
		size_t pos() const;

		template<typename T>
		void set(size_t offset, const T& value)
		{
			set(offset, (const byte*)&value, sizeof(T));
		}

		// Writes data into an old position
		void set(size_t offset, const byte* data, size_t len);

	private:
		// Size of a data slab. Whenever
		//  a slab runs out of data,
		//  a new slab is allocated.
		const size_t DATA_SIZE = 256;

		// Prior slabs
		std::vector<byte*> _slabs;

		// Current slab being written to
		byte* _currentSlab = nullptr;

		// Write head
		byte* _ptr = nullptr;
	};

	namespace compile
	{
		// Runs the 
		void run(const nlohmann::json&, std::ostream& out);
	}
}