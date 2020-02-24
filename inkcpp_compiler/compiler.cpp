#define INK_EXPOSE_JSON // make sure this is defined for internal compiler include
#include "compiler.h"
#include "binary_stream.h"

// STL includes
#include <string>
#include <stack>
#include <vector>
#include <map>
#include <set>
#include <fstream>

#define INK_COMPILER
#include "command.h"

namespace ink {

	namespace compiler
	{
		namespace internal
		{
			struct container_structure;

			struct compilation_data
			{
				binary_stream string_table;
				binary_stream container_data;
				std::vector<std::tuple<size_t, std::string, container_structure*, bool>> paths;

				// container map
				uint32_t next_container_index = 0;
				std::vector<std::pair<uint32_t, uint32_t>> container_map;

				void container_start(uint32_t offset, uint32_t containerIndex)
				{
					if (container_map.rbegin() != container_map.rend())
					{
						if (container_map.rbegin()->first > offset)
						{
							std::cerr << "WARNING: Container map written out of order. Wrote container at offset "
								<< offset << " after container with offset " << container_map.rbegin()->first << std::endl;
						}
					}

					container_map.push_back(std::make_pair(offset, containerIndex));
				}

				void container_end(uint32_t offset, uint32_t containerIndex)
				{
					if (container_map.rbegin() != container_map.rend())
					{
						if (container_map.rbegin()->first > offset)
						{
							std::cerr << "WARNING: Container map written out of order. Wrote container at offset " 
								<< offset << " after container with offset " << container_map.rbegin()->first << std::endl;
						}
					}

					container_map.push_back(std::make_pair(offset, containerIndex));
				}

				uint32_t push(const std::string& string)
				{
					// Save current position in table
					uint32_t pos = string_table.pos();

					// Write string to table (omit ^ if it begins with one)
					if (string.length() > 0 && string[0] == '^')
						string_table.write(string.substr(1));
					else
						string_table.write(string);

					// Return written position
					return pos;
				}
			};

			struct container_structure
			{
				std::vector<container_structure*> children;
				std::map<std::string, container_structure*> named_children;
				std::map<int, container_structure*> indexed_children;
				std::map<int, uint32_t> noop_offsets;
				container_structure* parent;
				uint32_t offset = 0;
				container_t counter_index = ~0;

				std::vector<std::tuple<nlohmann::json, std::string>> deferred;

				~container_structure()
				{
					// Destroy children
					for (container_structure* child : children)
					{
						delete child;
					}

					// Clear lists
					children.clear();
					named_children.clear();
					indexed_children.clear();
					noop_offsets.clear();
					parent = nullptr;
				}
			};

			template<typename T>
			void write(compilation_data& data, Command command, const T& param, CommandFlag flag = CommandFlag::NO_FLAGS)
			{
				static_assert(sizeof(T) == 4, "Command params must be 4-bytes");
				data.container_data.write(command);
				data.container_data.write(flag);
				data.container_data.write(param);
			}

			void write(compilation_data& data, Command command, CommandFlag flag = CommandFlag::NO_FLAGS)
			{
				data.container_data.write(command);
				data.container_data.write(flag);
			}

			void write_path(compilation_data& data, Command command, const std::string& path, container_structure* context, CommandFlag flags = CommandFlag::NO_FLAGS, bool useCountIndex = false)
			{
				// Write command out with stub param
				write(data, command, (uint32_t)0, flags);

				// Note to write over that param later
				size_t param_position = data.container_data.pos() - sizeof(uint32_t);
				data.paths.push_back(make_tuple(param_position, path, context, useCountIndex));
			}

			void write_variable(compilation_data& data, Command command, const std::string& name, CommandFlag flag = CommandFlag::NO_FLAGS)
			{
				// Hash the name of the variable
				uint32_t hash = hash_string(name.c_str());

				// Write it out
				write(data, command, hash, flag);

				// TODO: Check for hash collisions?
			}

			void compile_command(const std::string& name, compilation_data& data);
			container_structure* compile_container(const nlohmann::json& container, compilation_data& data, container_structure* parent, int index_in_parent);
			void process_paths(compilation_data& data, container_structure* root);

			uint32_t handle_settings(const nlohmann::json& meta, compilation_data& data, container_structure* self, container_structure* parent, bool& recordInContainerMap)
			{
				std::string name;
				uint32_t indexToReturn = ~0;
				recordInContainerMap = false;

				// Should be an assert
				if (meta.is_object())
				{
					for (auto& meta_iter : meta.items())
					{
						// Name
						if (meta_iter.key() == "#n")
						{
							// Add to parent's named children list
							if (parent != nullptr)
							{
								name = meta_iter.value().get<std::string>();
								parent->named_children.insert({ name, self });
							}
						}
						// Flags
						else if (meta_iter.key() == "#f")
						{
							int flags = meta_iter.value().get<int>();

							bool visits = false, turns = false, onlyFirst = false;

							if ((flags & 0x1) > 0) // Should record visit counts
							{
								visits = true;
							}
							if ((flags & 0x2) > 0) // Should record turn counts
							{
								turns = true;
							}
							if ((flags & 0x4) > 0) // Only count when you enter the first subelement
							{
								onlyFirst = true;
							}

							if (visits || turns)
							{
								uint32_t myIndex = data.next_container_index++;

								// Make appropriate flags
								CommandFlag flags = CommandFlag::NO_FLAGS;
								if (visits)
									flags |= CommandFlag::CONTAINER_MARKER_TRACK_VISITS;
								if(turns)
									flags |= CommandFlag::CONTAINER_MARKER_TRACK_TURNS;

								// Write command out at this position
								write(data, Command::START_CONTAINER_MARKER, myIndex, flags);

								indexToReturn = myIndex;
								self->counter_index = indexToReturn;

								//if (!onlyFirst)
								{
									recordInContainerMap = true;

									// TODO: Do we need to nudge the offset so that when we jump to this container
									//  we don't hit the marker? I think yes. Let's do it here.
									self->offset = data.container_data.pos();
									data.container_start(self->offset, myIndex);
								}
							}
						}
						// Child container
						else
						{
							// Add to deferred compilation list
							self->deferred.push_back(std::make_tuple(meta_iter.value(), meta_iter.key()));
						}
					}
				}

				return indexToReturn;
			}

			container_structure* compile_container(const nlohmann::json& container, compilation_data& data, container_structure* parent, int index_in_parent)
			{
				// Create meta structure and add to parent
				container_structure* self = new container_structure();
				self->parent = parent;
				self->offset = data.container_data.pos();
				if (parent != nullptr)
				{
					parent->children.push_back(self);
					parent->indexed_children.insert(std::make_pair(index_in_parent, self));
				}

				// Handle settings object
				bool recordInContainerMap = false;
				uint32_t myIndex = handle_settings(*container.rbegin(), data, self, parent, recordInContainerMap);

				int index = -1;

				auto end = container.end() - 1;
				for (auto iter = container.begin(); iter != end; ++iter)
				{
					// Increment index
					index++;

					// Recursive compilation
					if (iter->is_array())
						compile_container(*iter, data, self, index);
					else if (iter->is_string())
					{
						// Grab the string
						std::string string = iter->get<std::string>();

						// Check for string data
						if (string[0] == '^')
						{
							write(data, Command::STR, data.push(string));
						}
						else if (string == "nop")
						{
							// Add addressable reference
							self->noop_offsets.insert(std::make_pair(index, data.container_data.pos()));
						}
						else
						{
							compile_command(string, data);
						}
					}
					else if (iter->is_number()) // floats?
					{
						// Get the integer value
						int value = iter->get<int>();

						// Write int command
						write(data, Command::INT, value);
					}
					else if (iter->is_object())
					{
						// Divert
						if (iter->find("->") != iter->end())
						{
							// Get the divert path
							auto path = (*iter)["->"].get<std::string>();

							CommandFlag flag = CommandFlag::NO_FLAGS;
							auto isConditional = iter->find("c");
							if (isConditional != iter->end() && isConditional->get<bool>())
							{
								flag = CommandFlag::DIVERT_HAS_CONDITION;
							}

							// Is it a variable divert?
							auto isVar = iter->find("var");
							if (isVar != iter->end() && isVar->get<bool>())
							{
								write_variable(data, Command::DIVERT_TO_VARIABLE, path, flag);
							}
							else
							{
								// Write path in DIVERT command
								write_path(data, Command::DIVERT, path, self, flag);
							}
						}
						else if (iter->find("^->") != iter->end()) // divert to value
						{
							// Get the divert path
							auto path = (*iter)["^->"].get<std::string>();

							// Write path in DIVERT_VAL command
							write_path(data, Command::DIVERT_VAL, path, self);
						}
						else if (iter->find("->t->") != iter->end())
						{
							// Get the tunnel path
							auto path = (*iter)["->t->"].get<std::string>();

							// Tunnel command
							write_path(data, Command::TUNNEL, path, self);
						}
						// Temporary variable
						else if (iter->find("temp=") != iter->end())
						{
							// Get variable name
							auto name = (*iter)["temp="].get<std::string>();

							// Define temporary variable
							write_variable(data, Command::DEFINE_TEMP, name);
						}
						else if (iter->find("VAR=") != iter->end())
						{
							// Get variable name
							auto name = (*iter)["VAR="].get<std::string>();

							// check if it's a redefinition
							bool is_redef = false;
							auto re = iter->find("re");
							if (re != iter->end())
								is_redef = re->get<bool>();							

							// Set variable
							write_variable(data, Command::SET_VARIABLE, name, 
								is_redef ? CommandFlag::ASSIGNMENT_IS_REDEFINE : CommandFlag::NO_FLAGS);
						}
						// Push variable value onto stack
						else if (iter->find("VAR?") != iter->end())
						{
							// Get variable name
							auto name = (*iter)["VAR?"].get<std::string>();

							// Define temporary variable
							write_variable(data, Command::PUSH_VARIABLE_VALUE, name);
						}
						// Choice
						else if (iter->find("*") != iter->end())
						{
							// Get the choice path and flags
							auto path = (*iter)["*"].get<std::string>();
							int flags = (*iter)["flg"].get<int>();

							// Create choice command
							write_path(data, Command::CHOICE, path, self, (CommandFlag)flags);
						}
						// read count
						else if (iter->find("CNT?") != iter->end())
						{
							// Get the path to run a count check on
							auto path = (*iter)["CNT?"].get<std::string>();

							// Write out path. Speciically, we want the post-processor to write out the counter index for this container
							write_path(data, Command::READ_COUNT, path, self, CommandFlag::NO_FLAGS, true);
						}
						// external function call
						else if (iter->find("x()") != iter->end())
						{
							// Get name and argument count
							auto name = (*iter)["x()"].get<std::string>();
							int numArgs = 
								iter->find("exArgs") == iter->end()
								? 0
								: (*iter)["exArgs"].get<int>();

							// Encode num arguments into command flag and write out the hash of the function name as the parameter
							write(data, Command::CALL_EXTERNAL, hash_string(name.c_str()), (CommandFlag)numArgs);
						}
					}
				}

				if (self->deferred.size() > 0)
				{
					std::vector<size_t> divert_positions;

					// (1) Write empty divert
					write(data, Command::DIVERT, 0, CommandFlag::DIVERT_IS_FALLTHROUGH);
					divert_positions.push_back(data.container_data.pos() - sizeof(uint32_t));

					// (2) Write deffered containers
					for (auto& t : self->deferred)
					{
						using std::get;

						// Add to named child list
						container_structure* namedChild = compile_container(get<0>(t), data, self, -1);
						self->named_children.insert({ get<1>(t), namedChild });

						// Need a divert here
						write(data, Command::DIVERT, 0, CommandFlag::DIVERT_IS_FALLTHROUGH);
						divert_positions.push_back(data.container_data.pos() - sizeof(uint32_t));
					}

					// (3) Set divert positions
					for(size_t offset : divert_positions)
						data.container_data.set(offset, data.container_data.pos());
				}

				// (4) Record end offset (only if applicable)
				if (myIndex != ~0)
				{
					write(data, Command::END_CONTAINER_MARKER, myIndex);

					if (recordInContainerMap)
						data.container_end(data.container_data.pos(), myIndex);
				}
				return self;
			}

			void compile_command(const std::string& name, compilation_data& data)
			{
				for (int i = 0; i < (int)Command::NUM_COMMANDS; i++)
				{
					if (CommandStrings[i] != nullptr && name == CommandStrings[i])
					{
						write(data, (Command)i);
						return;
					}
				}

				// Error?
				std::cerr << "Unhandled command " << name << std::endl;
			}

			void process_paths(compilation_data& data, container_structure* root)
			{
				for (auto pair : data.paths)
				{
					// We need to replace the uint32_t at this location with the byte position of the requested container
					using std::get;
					size_t position = get<0>(pair);
					const std::string& path = get<1>(pair);
					container_structure* context = get<2>(pair);
					bool useCountIndex = get<3>(pair);

					// Start at the root
					container_structure* container = root;

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
					const char* token = strtok_s(const_cast<char*>(path_cstr), ".", &_context);
					while (token != nullptr)
					{
						// Number
						if (std::isdigit(token[0]))
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
							container = container->named_children[token];
						}

						firstParent = false;

						// Get the next token
						token = strtok_s(nullptr, ".", &_context);
					}

					if (noop_offset != ~0)
					{
						inkAssert(!useCountIndex, "Can't count visits to a noop!");
						data.container_data.set(position, noop_offset);
					}
					else
					{
						// If we want the count index, write that out
						if (useCountIndex)
						{
							inkAssert(container->counter_index != ~0, "No count index available for this container!");
							data.container_data.set(position, container->counter_index);
						}
						else
						{
							// Otherwise, write container address
							data.container_data.set(position, container->offset);
						}
					}
				}
			}

			void write_container_map(const compilation_data& data, std::ostream& out)
			{
				// Write out container count
				out.write((const char*)&data.next_container_index, sizeof(uint32_t));

				// Write out entries
				for (const auto& pair : data.container_map)
				{
					out.write((const char*)& pair.first, sizeof(uint32_t));
					out.write((const char*)& pair.second, sizeof(uint32_t));
				}
			}

			void write_container_hash_map(const std::string& name, const container_structure* context, std::ostream& out)
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
					write_container_hash_map(child_name, child.second, out);
				}

				for (auto child : context->indexed_children)
				{
					write_container_hash_map(name, child.second, out);
				}
			}

			void write_container_hash_map(const container_structure* data, std::ostream& out)
			{
				write_container_hash_map("", data, out);
			}

			void log_container_counter_index(const std::string& name, const container_structure* container)
			{
				if (container->counter_index != ~0)
				{
					std::clog << "Container " << name << " has counter index " << container->counter_index << std::endl;
				}

				std::set<const container_structure*> used;
				for (auto& pair : container->named_children)
				{
					log_container_counter_index(pair.first, pair.second);
					used.insert(pair.second);
				}

				for (auto child : container->children)
				{
					if (used.find(child) != used.end())
						continue;

					log_container_counter_index("", child);
				}
			}
		}

		void run(const nlohmann::json& src, std::ostream& out)
		{
			using namespace internal;

			// Get the runtime version
			int inkVersion = src["inkVersion"];

			// Create compilation data
			compilation_data data;

			// Get the root container and compile it
			container_structure* root = compile_container(src["root"], data, nullptr, 0);

			// Handle path processing
			process_paths(data, root);

			// == Start writing to the file ==

			// Write the ink version
			out.write((const char*)&inkVersion, sizeof(int));

			// Write the string table
			data.string_table.write_to(out);

			// Write a separator
			out << (char)0;

			// Write out container map
			write_container_map(data, out);

			// Write a separator
			uint32_t END_MARKER = ~0;
			out.write((const char*)& END_MARKER, sizeof(uint32_t));

			// Write container hash list
			write_container_hash_map(root, out);
			out.write((const char*)&END_MARKER, sizeof(uint32_t));

			// Write the container data
			data.container_data.write_to(out);

			// Flush the file
			out.flush();

			// LOGGING
			// log_container_counter_index("ROOT", root);

			delete root;
			root = nullptr;
		}

		void run(const char* filenameIn, const char* filenameOut)
		{
			// Load JSON
			nlohmann::json j;
			std::ifstream fin(filenameIn);
			fin >> j;

			// Open output stream
			std::ofstream fout(filenameOut, std::ios::binary | std::ios::out);

			// Run compiler
			ink::compiler::run(j, fout);

			// Close file
			fout.close();
		}

		void run(const char* filenameIn, std::ostream& out)
		{
			// Load JSON
			nlohmann::json j;
			std::ifstream fin(filenameIn);
			fin >> j;

			// Run compiler
			ink::compiler::run(j, out);
		}

		void run(const nlohmann::json& j, const char* filenameOut)
		{
			// Open output stream
			std::ofstream fout(filenameOut, std::ios::binary | std::ios::out);

			// Run compiler
			ink::compiler::run(j, fout);

			// Close file
			fout.close();
		}

		void run(std::istream& in, std::ostream& out)
		{
			// Load JSON
			nlohmann::json j;
			in >> j;

			// Run compiler
			ink::compiler::run(j, out);
		}

		void run(std::istream& in, const char* filenameOut)
		{
			// Load JSON
			nlohmann::json j;
			in >> j;

			// Open output stream
			std::ofstream fout(filenameOut, std::ios::binary | std::ios::out);

			// Run compiler
			ink::compiler::run(j, fout);

			// Close file
			fout.close();
		}

	}
}