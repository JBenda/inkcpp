/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include <system.h>
#include <config.h>
#include "command.h"
#include "types.h"
#include "story.h"
#include "header.h"
#include "list_table.h"

namespace ink::runtime::internal
{
// Ink story. Constant once constructed. Can be shared safely between multiple runner instances
class story_impl : public story
{
public:
#ifdef INK_ENABLE_STL
	story_impl(const char* filename);
#endif
	// Create story from allocated binary data in memory. If manage is true, this class will delete
	//  the pointers on destruction
	story_impl(const unsigned char* binary, size_t len, bool manage = true);
	virtual ~story_impl();

	const char* string(uint32_t index) const;

	inline ip_t instructions() const { return _instruction_data; }

	inline ip_t end() const { return _file + _length; }

	inline uint32_t num_containers() const { return _num_containers; }

	const list_flag* lists() const { return _lists; }

	const char* list_meta() const { return _list_meta; }

	size_t list_meta_size() const { return _list_meta_size; }

	// Find the innermost container containing offset. If offset is the start of a container, return
	// that container.
	container_t find_container_for(uint32_t offset) const;

	// Find the container which starts exactly at offset. Return false if this isn't the start of a
	// container.
	bool find_container_id(uint32_t offset, container_t& container_id) const;

	using container_data_t = ink::internal::container_data_t;
	using container_hash_t = ink::internal::container_hash_t;
	using container_map_t  = ink::internal::container_map_t;

	// Look up the details of the given container
	const container_data_t& container_data(container_t id) const
	{
		inkAssert(
		    id < _num_containers, "Container ID %u out of range (num_containers=%u)", ( unsigned ) id,
		    ( unsigned ) _num_containers
		);
		return _container_data[id];
	}

	// Look up the instruction pointer for the start of the given container
	ip_t container_offset(container_t id) const
	{
		return _instruction_data + container_data(id)._start_offset;
	}

	// Get container flag from container offset (either start or end)
	CommandFlag container_flag(ip_t offset) const;

	ip_t find_offset_for(hash_t path) const;

	// Find the hash to use for migration at the given instruction offset.
	// First tries an exact match in _container_hash (handles named but untracked containers such as
	// unlabeled choice bodies c-0, c-1, etc.), then falls back to find_container_for for positions
	// that are mid-content rather than at a container boundary.
	hash_t find_migration_hash(uint32_t offset) const;

	// Creates a new global store for use with runners executing this story
	virtual globals new_globals() override;
	virtual globals new_globals_from_snapshot(const snapshot&) override;
	virtual runner  new_runner(globals store = nullptr) override;
	virtual runner
	    new_runner_from_snapshot(const snapshot&, globals store = nullptr, unsigned idx = 0) override;

	hash_t hash() const override { return hash_data(_file, _length); }

private:
	void setup_pointers();

private:
	// file information
	const unsigned char* _file;
	size_t               _length;

	// string table
	const char* _string_table = nullptr;

	const char*      _list_meta      = nullptr;
	size_t           _list_meta_size = 0;
	const list_flag* _lists          = nullptr;

	// Information about containers.
	const container_data_t* _container_data = nullptr;
	uint32_t                _num_containers = 0;

	// How to find containers from instruction offsets.
	const container_map_t* _container_map      = nullptr;
	uint32_t               _container_map_size = 0;

	// How to find containers from string hashes.
	const container_hash_t* _container_hash      = nullptr;
	uint32_t                _container_hash_size = 0;

	// instruction info
	ip_t _instruction_data = nullptr;

	// story block used to create various weak pointers
	ref_block* _block;

	// whether we need to delete our binary data after we destruct
	bool _managed;
};
} // namespace ink::runtime::internal
