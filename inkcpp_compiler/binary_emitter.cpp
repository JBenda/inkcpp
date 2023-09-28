#include "binary_emitter.h"

#include "header.h"
#include "version.h"
#include "list_data.h"

#include <vector>
#include <map>
#include <fstream>

#ifndef WIN32
#include <cstring>
#endif

namespace ink::compiler::internal
{
	using std::vector;
	using std::map;
	using std::string;

	char* strtok_s(char * s, const char * sep, char** context) {
#if defined(_WIN32) || defined(_WIN64)
		return ::strtok_s(s, sep, context);
#else
		if (
				context == nullptr ||
				sep == nullptr ||
				(s == nullptr && *context == nullptr) )
		{
			errno = EINVAL;
			return nullptr;
		}
		return ::strtok_r(s, sep, context);
#endif
	}

	// holds information about a container
	struct container_data
	{
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
		uint32_t offset = 0;
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
			//named_children.clear();
			//indexed_children.clear();
			//noop_offsets.clear();
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
		container->offset = _containers.pos();

		// Add to parents lists
		if (_current != nullptr)
		{
			_current->children.push_back(container);
			_current->indexed_children.insert({ index_in_parent, container });

			if (!name.empty()) {
				_current->named_children.insert({ name, container });
			}
		}

		// Set this as the current pointer
		_current = container;

		// Return current position
		return _containers.pos();
	}

	uint32_t binary_emitter::end_container()
	{
		// Move up the chain
		_current->end_offset = _containers.pos();
		_current = _current->parent;

		// Return offset
		return _containers.pos();
	}

   	int binary_emitter::function_container_arguments(const std::string& name)
	{
		if(_root == nullptr) { return -1; }
		auto fn = _root->named_children.find(name);
		if (fn == _root->named_children.end()) { return -1; }

		size_t offset = fn->second->offset;
		byte_t cmd = _containers.get(offset);
		int arity = 0;
		while(static_cast<Command>(cmd) == Command::DEFINE_TEMP) {
			offset += 6; // command(1) + flag(1) + variable_name_hash(4)
			cmd = _containers.get(offset);
			++arity;		  
		}
		return arity;
	}

	void binary_emitter::write_raw(Command command, CommandFlag flag, const char* payload, ink::size_t payload_size)
	{
		_containers.write(command);
		_containers.write(flag);
		if(payload_size > 0)
			_containers.write((const byte_t*)payload, payload_size);
	}

	void binary_emitter::write_path(Command command, CommandFlag flag, const std::string& path, bool useCountIndex)
	{
		// Write blank command with 0 payload
		write(command, (uint32_t)0, flag);

		// Note the position of this later so we can resolve the paths at the end
		size_t param_position = _containers.pos() - sizeof(uint32_t);
		bool op = flag & CommandFlag::FALLBACK_FUNCTION;
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

	void binary_emitter::write_list(Command command, CommandFlag flag, const std::vector<list_flag>& entries) {
		uint32_t id = _list_count++;
		for(const list_flag& entry : entries) {
			_lists.write(entry);
		}
		_lists.write(null_flag);
		write(command, id, flag);
	}

	void binary_emitter::handle_nop(int index_in_parent)
	{
		_current->noop_offsets.insert({ index_in_parent, _containers.pos() });
	}

	void binary_emitter::output(std::ostream& out)
	{
		// Write the ink version
		// TODO: define this order in header?
		using header = ink::internal::header;
		header::endian_types same = header::endian_types::same;
		out.write((const char*)&same, sizeof(decltype(same)));
		out.write((const char*)&_ink_version, sizeof(decltype(_ink_version)));
		out.write((const char*)&ink::InkBinVersion, sizeof(decltype(ink::InkBinVersion)));

		// Write the string table
		_strings.write_to(out);

		// Write a separator
		out << (char)0;

		// Write lists meta data and defined lists
		_lists.write_to(out);
		// Write a seperator
		out.write(reinterpret_cast<const char*>(&null_flag), sizeof(null_flag));

		// Write out container map
		write_container_map(out, _container_map, _max_container_index);

		// Write a separator
		uint32_t END_MARKER = ~0;
		out.write((const char*)&END_MARKER, sizeof(uint32_t));

		// Write container hash list
		write_container_hash_map(out);
		out.write((const char*)&END_MARKER, sizeof(uint32_t));

		// Write the container data
		_containers.write_to(out);

		// Flush the file
		out.flush();
	}

	void binary_emitter::initialize()
	{
		// Reset binary data stores
		_strings.reset();
		_list_count = 0;
		_lists.reset();
		_containers.reset();

		// clear other data
		_paths.clear();

		if (_root != nullptr)
			delete _root;

		_current = nullptr;
		_root = nullptr;
	}

	void binary_emitter::finalize()
	{
		// post process path commands
		process_paths();
	}

	void binary_emitter::setContainerIndex(container_t index)
	{
		_current->counter_index = index;
	}

	uint32_t binary_emitter::fallthrough_divert()
	{
		// write a fallthrough divert ???
		write<uint32_t>(Command::DIVERT, (uint32_t)0, CommandFlag::DIVERT_IS_FALLTHROUGH);

		// Return the location of the divert offset
		return _containers.pos() - sizeof(uint32_t);
	}

	void binary_emitter::patch_fallthroughs(uint32_t position)
	{
		// Patch
		_containers.set(position, _containers.pos());
	}

	void binary_emitter::process_paths()
	{
		for (auto pair : _paths)
		{
			// We need to replace the uint32_t at this location with the byte position of the requested container
			using std::get;
			size_t position = get<0>(pair);
			const std::string& path = get<1>(pair);
			bool optional = get<2>(pair);
			container_data* context = get<3>(pair);
			bool useCountIndex = get<4>(pair);

			// Start at the root
			container_data* container = _root;

			// Unless it's a relative path
			const char* path_cstr = path.c_str();
			if (path_cstr[0] == '.')
			{
				container = context;
				path_cstr += 1;
			}

			bool firstParent = true;

			// We need to parse the path
			offset_t noop_offset = ~0;
			char* _context = nullptr;
			const char* token = ink::compiler::internal::strtok_s(
					const_cast<char*>(path_cstr), ".", &_context);
			while (token != nullptr)
			{
				// Number
				// variable names can start with a number
				bool isNumber = true;
				for(const char* i = token; *i; ++i) {
					if(!isdigit(*i)) { isNumber = false; break; }
				}
				if(isNumber)
				{
					// Check if we have a nop registered at that index
					int index = atoi(token);
					auto nop_iter = container->noop_offsets.find(index);
					if (nop_iter != container->noop_offsets.end())
					{
						noop_offset = nop_iter->second;
						break;
					}
					else
						container = container->indexed_children[index];
				}
				// Parent
				else if (token[0] == '^')
				{
					if (!firstParent)
						container = container->parent;
				}
				// Named child
				else
				{
					auto itr = container->named_children.find(token);
					container = itr == container->named_children.end()
						? nullptr
						: itr->second;
				}

				firstParent = false;

				// Get the next token
				token = ink::compiler::internal::strtok_s(nullptr, ".", &_context);
			}

			if (noop_offset != ~0)
			{
				inkAssert(!useCountIndex, "Can't count visits to a noop!");
				_containers.set(position, noop_offset);
			}
			else
			{
				// If we want the count index, write that out
				if (useCountIndex)
				{
					inkAssert(container->counter_index != ~0, "No count index available for this container!");
					_containers.set(position, container->counter_index);
				}
				else
				{
					// Otherwise, write container address
					if (container == nullptr) {
						_containers.set(position, 0);
						inkAssert(optional, "Was not able to resolve a not optional path! '%s'", path.c_str());
					} else {
						_containers.set(position, container->offset);
					}
				}
			}
		}
	}

	void binary_emitter::write_container_map(std::ostream& out, const container_map& map, container_t num)
	{
		// Write out container count
		out.write(reinterpret_cast<const char*>(&num), sizeof(container_t));

		// Write out entries
		for (const auto& pair : map)
		{
			out.write((const char*)&pair.first, sizeof(uint32_t));
			out.write((const char*)&pair.second, sizeof(uint32_t));
		}
	}

	void binary_emitter::write_container_hash_map(std::ostream& out)
	{
		write_container_hash_map(out, "", _root);
	}

	void binary_emitter::write_container_hash_map(std::ostream& out, const std::string& name, const container_data* context)
	{
		for (auto child : context->named_children)
		{
			// Get the child's name in the hierarchy
			std::string child_name = name.empty() ? child.first : (name + "." + child.first);
			hash_t name_hash = hash_string(child_name.c_str());

			// Write out name hash and offset
			out.write((const char*)&name_hash, sizeof(hash_t));
			out.write((const char*)&child.second->offset, sizeof(uint32_t));

			// Recurse
			write_container_hash_map(out, child_name, child.second);
		}

		for (auto child : context->indexed_children)
		{
			write_container_hash_map(out, name, child.second);
		}
	}

	void binary_emitter::set_list_meta(const list_data &list_defs) {
		if (list_defs.empty()) {
			return;
		}

		auto flags = list_defs.get_flags();
		auto list_names = list_defs.get_list_names().begin();
		int list_id = -1;
		for(const auto& flag : flags) {
			_lists.write(flag.flag);
			if(flag.flag.list_id != list_id) {
				list_id = flag.flag.list_id;
				_lists.write(reinterpret_cast<const byte_t*>(list_names->data()), list_names->size());
				++list_names;
				_lists.write('\0');
			}
			_lists.write(reinterpret_cast<const byte_t*>(flag.name.c_str()), flag.name.size() + 1);
		}
		_lists.write(null_flag);
	}
}
