/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "system.h"
#include "command.h"

namespace ink::internal
{

struct header {

	static constexpr uint32_t InkBinMagic        = ('I' << 24) | ('N' << 16) | ('K' << 8) | 'B';
	static constexpr uint32_t InkBinMagic_Differ = ('B' << 24) | ('K' << 16) | ('N' << 8) | 'I';
	static constexpr uint32_t Alignment          = 16;

	uint32_t ink_bin_magic          = InkBinMagic;
	uint16_t ink_version_number     = 0;
	uint16_t ink_bin_version_number = 0;

	enum class endian_types : uint8_t {
		none,
		same,
		differ
	};

	constexpr endian_types endian() const
	{
		switch (ink_bin_magic) {
			case InkBinMagic: return endian_types::same;
			case InkBinMagic_Differ: return endian_types::differ;
			default: return endian_types::none;
		}
	}

	bool validate() const;

	struct section_t {
		uint32_t _start = 0;
		uint32_t _bytes = 0;

		void setup(uint32_t& offset, uint32_t bytes)
		{
			_start = (offset + Alignment - 1) & ~(Alignment - 1);
			_bytes = bytes;
			offset = _start + _bytes;
		}
	};

	// File section sizes. Each section is aligned to Alignment
	section_t _strings;
	section_t _list_meta;
	section_t _lists;
	section_t _containers;
	section_t _container_map;
	section_t _container_hash;
	section_t _instructions;
};

// One entry in the container hash. Used to translate paths into story locations.
struct container_hash_t {
	// Hash of the container's path string.
	hash_t _hash;

	// Offset to the start of this container.
	uint32_t _offset;

	uint32_t key() const { return _hash; }

	/* Ordered by hash, then by offset.
	 *
	 * The offset is not decoration: without it this is a partial order, and
	 * binary_emitter sorts the table with std::sort, which is not stable. Any two
	 * entries sharing a hash could then come out in either order, and which one you
	 * got depended on the standard library - the same story compiled to different
	 * bytes under libstdc++ than under libc++, which makes byte comparison useless
	 * as a check between builds.
	 *
	 * It is not only cosmetic. story_impl.cpp's upper_bound() returns the LAST entry
	 * whose key is <= the target, so when entries share a hash the runtime resolves
	 * the path to whichever one the sort happened to leave last. Ties are common:
	 * paths are not unique here, because build_container_hash_map() recurses into
	 * indexed children carrying the parent's name unchanged, so one path string is
	 * emitted once per indexed child at a different offset. One test story has 164
	 * entries under 128 distinct hashes, and every one of those 26 collisions is a
	 * repeated path rather than two different paths colliding.
	 *
	 * Tie-breaking on the offset makes the order total, so every toolchain agrees
	 * and the resolved container is a property of the story rather than of the
	 * compiler that built it. key() deliberately still returns the hash alone - the
	 * runtime's binary search looks up by hash and must keep matching every entry in
	 * a run. */
	bool operator<(const container_hash_t& other) const
	{
		return _hash != other._hash ? _hash < other._hash : _offset < other._offset;
	}
};

// One entry in the container map. Used to work out which container a story location is in.
struct container_map_t {
	// Offset to the start of this container's instructions.
	uint32_t _offset;

	// Container index.
	container_t _id;

	uint32_t key() const { return _offset; }

	bool operator<(const container_map_t& other) const { return _offset < other._offset; }
};

// One entry in the container data. Describes containers.
struct container_data_t {
	/// Parent container, or ~0 if this is the root.
	// TODO: Pack into 28 with explicit invalid container_t, since we expect fewer containers than
	// instructions.
	container_t _parent;

	/// Container flags (saves looking up via instruction data)
	uint32_t _flags : 4;

	/// Instruction offset to the start instruction (enter marker) of this container.
	uint32_t _start_offset : 28;

	/// Instruction offset to the end instruction (leave marker) of this container
	uint32_t _end_offset;

	/// Container hash.
	uint32_t _hash;

	/// Check to see if the instruction offset is part of the instructions for this container. Note
	/// that this is inclusive not exclusive.
	bool contains(uint32_t offset) const { return offset >= _start_offset && offset <= _end_offset; }

	/// Check to see if this is a knot container.
	bool knot() const { return _flags & uint8_t(CommandFlag::CONTAINER_MARKER_IS_KNOT); }

	/// Check to see if this is a container which tracks visits.
	bool visit() const { return _flags & uint8_t(CommandFlag::CONTAINER_MARKER_TRACK_VISITS); }
};
} // namespace ink::internal
