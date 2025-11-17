/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "system.h"
#include "command.h"

namespace ink::internal {

		struct header {
			static header parse_header(const char* data);

			template<typename T>
			static T swap_bytes(const T& value) {
				char data[sizeof(T)];
				for (int i = 0; i < sizeof(T); ++i) {
					data[i] = reinterpret_cast<const char*>(&value)[sizeof(T)-1-i];
				}
				return *reinterpret_cast<const T*>(data);
			}
			list_flag read_list_flag(const char*& ptr) const {
				list_flag result = *reinterpret_cast<const list_flag*>(ptr);
				ptr += sizeof(list_flag);
				if (endien == ink::internal::header::endian_types::differ) {
					result.flag = swap_bytes(result.flag);
					result.list_id = swap_bytes(result.list_id);
				}
				return result;
			}

			enum class  endian_types: uint16_t {
				none = 0,
				same = 0x0001,
				differ = 0x0100
			} endien = endian_types::none;
			uint32_t ink_version_number = 0;
			uint32_t ink_bin_version_number = 0;
			static constexpr size_t Size = ///< actual data size of Header,
										   ///   because padding of struct may
										   ///   differ between platforms
				sizeof(uint16_t) + 2 * sizeof(uint32_t);
		};

		// One entry in the container hash. Used to translate paths into story locations.
		struct container_hash_t
		{
			// Hash of the container's path string.
			hash_t _hash;

			// Offset to the start of this container.
			uint32_t _offset;

			uint32_t key() const { return _hash; }
			bool operator<(const container_hash_t& other) const { return _hash < other._hash; }
		};

		// One entry in the container map. Used to work out which container a story location is in.
		struct container_map_t
		{
			// Offset to the start of this container's instructions.
			uint32_t _offset;

			// Container index.
			container_t _id;

			uint32_t key() const { return _offset; }
			bool operator<(const container_map_t& other) const { return _offset < other._offset; }
		};

		// One entry in the container data. Describes containers.
		struct container_data_t
		{
			/// Parent container, or ~0 if this is the root.
			// TODO: Pack into 28 with explicit invalid container_t, since we expect fewer containers than instructions.
			container_t _parent;

			/// Container flags (saves looking up via instruction data)
			CommandFlag _flags : 4;

			/// Instruction offset to the start instruction (enter marker) of this container.
			uint32_t _start_offset : 28;

			/// Instruction offset to the end instruction (leave marker) of this container
			uint32_t _end_offset;

			/// Container hash.
			uint32_t _hash;

			/// Check to see if the instruction offset is part of the instructions for this container. Note that this is inclusive not exclusive.
			bool contains(uint32_t offset) const { return offset >= _start_offset && offset <= _end_offset; }
		};
}
