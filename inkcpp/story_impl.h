#pragma once

#include <system.h>
#include <config.h>
#include "types.h"
#include "story.h"

namespace ink::runtime::internal
{
	// Ink story. Constant once constructed. Can be shared safely between multiple runner instances
	class story_impl : public story
	{
	public:
		struct Header {
			static Header parseHeader(const char* data);

			template<typename T>
			static T swapBytes(T value) {
				char data[sizeof(T)];
				for (int i = 0; i < sizeof(T); ++i) {
					data[i] = reinterpret_cast<char*>(&value)[sizeof(T)-1-i];
				}
				return *reinterpret_cast<T*>(data);
			}

			enum class ENDENSE : uint16_t {
				NONE = 0,
				SAME = 0x0001,
				DIFFER = 0x0100
			} endien = ENDENSE::NONE;
			uint32_t inkVersionNumber = 0;
			uint32_t inkCppVersionNumber = 0;
		};

#ifdef INK_ENABLE_STL
		story_impl(const char* filename);
#endif
		// Create story from allocated binary data in memory. If manage is true, this class will delete
		//  the pointers on destruction
		story_impl(unsigned char* binary, size_t len, bool manage = true);
		virtual ~story_impl();

		const char* string(uint32_t index) const;
		inline const ip_t instructions() const { return _instruction_data; }
		inline const ip_t end() const { return _file + _length; }

		inline uint32_t num_containers() const { return _num_containers; }

		bool iterate_containers(const uint32_t*& iterator, container_t& index, ip_t& offset, bool reverse = false) const;
		bool get_container_id(ip_t offset, container_t& container_id) const;

		ip_t find_offset_for(hash_t path) const;

		// Creates a new global store for use with runners executing this story
		virtual globals new_globals() override;
		virtual runner new_runner(globals store = nullptr) override;

	private:
		void setup_pointers();

	private:
		// file information
		unsigned char* _file;
		size_t _length;

		Header _header;

		// string table
		const char* _string_table;

		// container info
		uint32_t* _container_list;
		uint32_t _container_list_size;
		uint32_t _num_containers;

		// container hashes
		hash_t* _container_hash_start;
		hash_t* _container_hash_end;

		// instruction info
		ip_t _instruction_data;

		// story block used to creat various weak pointers
		ref_block* _block;

		// whether we need to delete our binary data after we destruct
		bool _managed;
	};
}
