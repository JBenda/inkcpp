/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "binary_emitter.h"

#include "header.h"
#include "version.h"
#include "list_data.h"

#include <vector>
#include <map>
#include <fstream>
#include <algorithm>

#ifndef _MSC_VER
#	include <cstring>
#endif

namespace ink::compiler::internal
{
using std::vector;
using std::map;
using std::string;

char* strtok_s(char* s, const char* sep, char** context)
{
#ifdef _MSC_VER
	return ::strtok_s(s, sep, context);
#else
	if (context == nullptr || sep == nullptr || (s == nullptr && *context == nullptr)) {
		errno = EINVAL;
		return nullptr;
	}
	return ::strtok_r(s, sep, context);
#endif
}

// holds information about a container
struct container_data {
	// child containers
	vector<container_data*> children;

	// children names (not all containers have names)
	map<string, container_data*> named_children;

	// Indexes of child containers (index is child index of parent)
	map<int, container_data*> indexed_children;

	// Offsets of noop operations
	map<int, uint32_t> noop_offsets;

	// parent pointer
	container_data* parent = nullptr;

	// Offset in the binary stream
	uint32_t offset     = 0;
	uint32_t end_offset = 0;

	// Index used in CNT? operations
	container_t counter_index = ~0;

	~container_data()
	{
		// Destroy children
		for (auto child : children)
			delete child;

		// Clear lists
		children.clear();
		// named_children.clear();
		// indexed_children.clear();
		// noop_offsets.clear();
		parent = nullptr;
	}
};

binary_emitter::binary_emitter()
    : _root(nullptr)
{
}

binary_emitter::~binary_emitter()
{
	if (_root != nullptr)
		delete _root;
	_root = nullptr;
}

uint32_t binary_emitter::start_container(int index_in_parent, const std::string& name)
{
	// Create new container metadata
	auto container = new container_data();

	// Store root
	if (_root == nullptr)
		_root = container;

	// Parent it to the current container
	container->parent = _current;

	// Set offset to the current position
	container->offset = _instructions.pos();

	// Add to parents lists
	if (_current != nullptr) {
		_current->children.push_back(container);
		_current->indexed_children.insert({index_in_parent, container});

		if (! name.empty()) {
			_current->named_children.insert({name, container});
		}
	}

	// Set this as the current pointer
	_current = container;

	// Return current position
	return _instructions.pos();
}

uint32_t binary_emitter::end_container()
{
	// Move up the chain
	_current->end_offset = _instructions.pos();
	_current             = _current->parent;

	// Return offset
	return _instructions.pos();
}

int binary_emitter::function_container_arguments(const std::string& name)
{
	if (_root == nullptr) {
		return -1;
	}
	auto fn = _root->named_children.find(name);
	if (fn == _root->named_children.end()) {
		return -1;
	}

	size_t offset = fn->second->offset;
	byte_t cmd    = _instructions.get(offset);
	int    arity  = 0;
	while (static_cast<Command>(cmd) == Command::DEFINE_TEMP) {
		offset += 6; // command(1) + flag(1) + variable_name_hash(4)
		cmd = _instructions.get(offset);
		++arity;
	}
	return arity;
}

void binary_emitter::write_raw(
    Command command, CommandFlag flag, const char* payload, ink::size_t payload_size
)
{
	_instructions.write(command);
	_instructions.write(flag);
	if (payload_size > 0)
		_instructions.write(( const byte_t* ) payload, payload_size);
}

void binary_emitter::write_path(
    Command command, CommandFlag flag, const std::string& path, bool useCountIndex
)
{
	// Write blank command with 0 payload
	write(command, ( uint32_t ) 0, flag);

	// Note the position of this later so we can resolve the paths at the end
	size_t param_position = _instructions.pos() - sizeof(uint32_t);
	bool   op             = flag & CommandFlag::FALLBACK_FUNCTION;
	_paths.push_back(std::make_tuple(param_position, path, op, _current, useCountIndex));
}

void binary_emitter::write_variable(Command command, CommandFlag flag, const std::string& name)
{
	// Use hash as identifier
	uint32_t hash = hash_string(name.c_str());

	// Write it out
	write(command, hash, flag);
}

void binary_emitter::write_string(Command command, CommandFlag flag, const std::string& string)
{
	// Save current position in table
	uint32_t pos = _strings.pos();

	// Write string to table (omit ^ if it begins with one)
	if (string.length() > 0 && string[0] == '^')
		_strings.write(string.substr(1));
	else
		_strings.write(string);

	// Written position is what we write out in our command
	write(command, pos, flag);
}

void binary_emitter::write_list(
    Command command, CommandFlag flag, const std::vector<list_flag>& entries
)
{
	uint32_t id = _list_count++;
	for (const list_flag& entry : entries) {
		_lists.write(entry);
	}
	_lists.write(null_flag);
	write(command, id, flag);
}

void binary_emitter::handle_nop(int index_in_parent)
{
	_current->noop_offsets.insert({index_in_parent, _instructions.pos()});
}

template<typename type>
void binary_emitter::emit_section(std::ostream& stream, const std::vector<type>& data) const
{
	stream.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(type));
	close_section(stream);
}

void binary_emitter::emit_section(std::ostream& stream, const binary_stream& data) const
{
	inkAssert((stream.tellp() & (ink::internal::header::Alignment - 1)) == 0);
	data.write_to(stream);
	close_section(stream);
}

void binary_emitter::close_section(std::ostream& stream) const
{
	// Write zeroes until aligned.
	while (! stream.fail() && (stream.tellp() % ink::internal::header::Alignment))
		stream.put('\0');
}

void binary_emitter::output(std::ostream& out)
{
	// Create container data
	std::vector<container_data_t> container_data;
	container_data.resize(_max_container_index);
	build_container_data(container_data, ~0, _root);

	// Create container hash (and write the hashes into the data as well)
	std::vector<container_hash_t> container_hash;
	container_hash.reserve(_max_container_index);
	build_container_hash_map(container_hash, container_data, "", _root);

	// Sort map on ascending hash code.
	std::sort(container_hash.begin(), container_hash.end());

	// If there's list meta data...
	if (_list_meta.pos() > 0) {
		// If there are any lists, terminate the data correctly. Otherwise leave an empty section.
		if (_lists.pos() > 0)
			_lists.write(null_flag);
	} else
		// No meta data -> no lists.
		_lists.reset();

	// Fill in header
	ink::internal::header header;
	header.ink_version_number     = _ink_version;
	header.ink_bin_version_number = ink::InkBinVersion;

	// Fill in sections
	uint32_t offset = sizeof(header);
	header._strings.setup(offset, _strings.pos());
	header._list_meta.setup(offset, _list_meta.pos());
	header._lists.setup(offset, _lists.pos());
	header._containers.setup(offset, container_data.size() * sizeof(container_data_t));
	header._container_map.setup(offset, _container_map.size() * sizeof(container_map_t));
	header._container_hash.setup(offset, container_hash.size() * sizeof(container_hash_t));
	header._instructions.setup(offset, _instructions.pos());

	// Write the header
	out.write(reinterpret_cast<const char*>(&header), sizeof(header));
	close_section(out);

	// Write the string table
	emit_section(out, _strings);

	// Write lists meta data and defined lists
	emit_section(out, _list_meta);

	// Write lists meta data and defined lists
	emit_section(out, _lists);

	// Write out container information
	emit_section(out, container_data);

	// Write out container map
	emit_section(out, _container_map);

	// Write container hash list
	emit_section(out, container_hash);

	// Write the container contents (instruction stream)
	emit_section(out, _instructions);

	// Flush the file
	out.flush();
}

void binary_emitter::initialize()
{
	// Reset binary data stores
	_strings.reset();
	_list_count = 0;
	_list_meta.reset();
	_lists.reset();
	_instructions.reset();

	// clear other data
	_paths.clear();

	if (_root != nullptr)
		delete _root;

	_current = nullptr;
	_root    = nullptr;
}

void binary_emitter::finalize()
{
	// post process path commands
	process_paths();
}

void binary_emitter::setContainerIndex(container_t index) { _current->counter_index = index; }

uint32_t binary_emitter::fallthrough_divert()
{
	// write a fallthrough divert ???
	write<uint32_t>(Command::DIVERT, ( uint32_t ) 0, CommandFlag::DIVERT_IS_FALLTHROUGH);

	// Return the location of the divert offset
	return _instructions.pos() - sizeof(uint32_t);
}

void binary_emitter::patch_fallthroughs(uint32_t position)
{
	// Patch
	_instructions.set(position, _instructions.pos());
}

void binary_emitter::process_paths()
{
	for (auto pair : _paths) {
		// We need to replace the uint32_t at this location with the byte position of the requested
		// container
		using std::get;
		size_t             position      = get<0>(pair);
		const std::string& path          = get<1>(pair);
		bool               optional      = get<2>(pair);
		container_data*    context       = get<3>(pair);
		bool               useCountIndex = get<4>(pair);

		// Start at the root
		container_data* container = _root;

		// Unless it's a relative path
		const char* path_cstr = path.c_str();
		if (path_cstr[0] == '.') {
			container = context;
			path_cstr += 1;
		}

		bool firstParent = true;

		// We need to parse the path
		offset_t    noop_offset = ~0;
		char*       _context    = nullptr;
		const char* token
		    = ink::compiler::internal::strtok_s(const_cast<char*>(path_cstr), ".", &_context);
		while (token != nullptr) {
			// Number
			// variable names can start with a number
			bool isNumber = true;
			for (const char* i = token; *i; ++i) {
				if (! isdigit(*i)) {
					isNumber = false;
					break;
				}
			}
			if (isNumber) {
				// Check if we have a nop registered at that index
				int  index    = atoi(token);
				auto nop_iter = container->noop_offsets.find(index);
				if (nop_iter != container->noop_offsets.end()) {
					noop_offset = nop_iter->second;
					break;
				} else
					container = container->indexed_children[index];
			}
			// Parent
			else if (token[0] == '^') {
				if (! firstParent)
					container = container->parent;
			}
			// Named child
			else {
				auto itr  = container->named_children.find(token);
				container = itr == container->named_children.end() ? nullptr : itr->second;
			}

			firstParent = false;

			// Get the next token
			token = ink::compiler::internal::strtok_s(nullptr, ".", &_context);
		}

		if (noop_offset != ~0) {
			inkAssert(! useCountIndex, "Can't count visits to a noop!");
			_instructions.set(position, noop_offset);
		} else {
			// If we want the count index, write that out
			if (useCountIndex) {
				inkAssert(container->counter_index != ~0, "No count index available for this container!");
				_instructions.set(position, container->counter_index);
			} else {
				// Otherwise, write container address
				if (container == nullptr) {
					_instructions.set(position, 0);
					inkAssert(optional, "Was not able to resolve a not optional path! '%hs'", path.c_str());
				} else {
					_instructions.set(position, container->offset);
				}
			}
		}
	}
}

void binary_emitter::build_container_data(
    std::vector<container_data_t>& data, container_t parent, const container_data* context
) const
{
	// Build data for this container
	if (context->counter_index != ~0) {
		container_data_t& d = data[context->counter_index];
		d._parent           = parent;
		d._start_offset     = context->offset;
		d._end_offset       = context->end_offset;
		const uint8_t flags = _instructions.get(context->offset + 1);
		inkAssert(flags < 16);
		d._flags = flags;

		// Since we might be skipping tree levels, we need to be explicit about the parent.
		parent = context->counter_index;
	}

	// Recurse
	for (auto child : context->children)
		build_container_data(data, parent, child);
}

void binary_emitter::build_container_hash_map(
    std::vector<container_hash_t>& hash_map, std::vector<container_data_t>& data,
    const std::string& name, const container_data* context
) const
{
	// Search named children first.
	for (auto child : context->named_children) {
		// Get the child's name in the hierarchy
		std::string child_name = name.empty() ? child.first : (name + "." + child.first);

		// Hash name. We only do this at the named child level. In theory we could support indexed
		// children as well. The root is anonymous so the fact that it's skipped is not an issue.
		const hash_t child_name_hash = hash_string(child_name.c_str());

		// Store hash in the data.
		if (child.second->counter_index != ~0) {
			data[child.second->counter_index]._hash = child_name_hash;
		}

		// Append the name hash and offset
		hash_map.push_back({child_name_hash, child.second->offset});

		// Recurse
		build_container_hash_map(hash_map, data, child_name, child.second);
	}

	// Search indexed children (which duplicates named childen...)
	// TODO: Merge duplicate child arrays, very error-prone.
	for (auto child : context->indexed_children) {
		build_container_hash_map(hash_map, data, name, child.second);
	}
}

void binary_emitter::set_list_meta(const list_data& list_defs)
{
	if (list_defs.empty()) {
		return;
	}

	auto flags      = list_defs.get_flags();
	auto list_names = list_defs.get_list_names().begin();
	int  list_id    = -1;
	for (const auto& flag : flags) {
		_list_meta.write(flag.flag);
		if (flag.flag.list_id != list_id) {
			list_id = flag.flag.list_id;
			_list_meta.write(reinterpret_cast<const byte_t*>(list_names->data()), list_names->size());
			++list_names;
			_list_meta.write('\0');
		}
		_list_meta.write(reinterpret_cast<const byte_t*>(flag.name->c_str()), flag.name->size() + 1);
	}
	_list_meta.write(null_flag);
}
} // namespace ink::compiler::internal
