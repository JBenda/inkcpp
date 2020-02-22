#pragma once

#include "value.h"
#include "platform.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			class basic_stream
			{
			protected:
				basic_stream(data*, size_t);
			public:
				// Append data to stream
				void append(const data&);

				// Append data array to stream
				void append(const data*, unsigned int length);

				// Append fixed sized data array to stream
				template<unsigned int N>
				void append(const data in[N])
				{
					append(&in[0], N);
				}

				// Returns the number of data items that will be extracted by the next get
				int queued() const;

				// Peeks the top entry
				const data& peek() const;

				// discards data
				void discard(size_t length);

				// Extract into a data array
				void get(data*, size_t length);

				// Extract to a newly allocated string
				const char* get_alloc(string_table&);

#ifdef INK_ENABLE_STL
				// Extract into a string
				std::string get();
#endif
#ifdef INK_ENABLE_UNREAL
				// Extract into a string
				FString get();
#endif

				// Check if the stream is empty
				bool is_empty() const { return _size == 0; }

				// Check if the stream has a marker
				bool has_marker() const;

				// Checks if the stream ends with a specific type
				bool ends_with(data_type) const;

				// Checks if the last element when save()'d was this type
				bool saved_ends_with(data_type) const;

				// Checks if there are any elements past the save that
				//  are non-whitespace strings
				bool text_past_save() const;

				// Clears the whole stream
				void clear();

				// Marks strings that are in use
				void mark_strings(string_table&);

				// = Save/Restore
				void save();
				void restore();
				void forget();

			private:
				size_t find_start() const;
				bool should_skip(size_t iter, bool& hasGlue) const;
				
			private:
				// data stream
				data* _data;
				size_t _max;

				// size
				size_t _size;

				// save point
				size_t _save;
			};

#ifdef INK_ENABLE_STL
			std::ostream& operator <<(std::ostream&, basic_stream&);
			basic_stream& operator >>(basic_stream&, std::string&);
#endif
#ifdef INK_ENABLE_UNREAL
			basic_stream& operator >>(basic_stream&, FString&);
#endif

			basic_stream& operator<<(basic_stream&, const data&);

			const data marker = { data_type::marker, 0 };
			const data newline = { data_type::newline, 0 };
			const data glue = { data_type::glue, 0 };

			template<size_t N>
			class stream : public basic_stream
			{
			public:
				stream() : basic_stream(&_buffer[0], N) { }

			private:
				data _buffer[N];
			};
		}
	}
}
