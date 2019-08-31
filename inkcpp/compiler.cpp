// STL includes
#include <string>
#include <stack>
#include <vector>
#include <map>
#include <fstream>

#include "compiler.h"
#include "binary_stream.h"

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
				std::vector<std::tuple<size_t, std::string, container_structure*>> paths;
				std::stack<std::tuple<nlohmann::json, std::string, container_structure*>> deferred;

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
				container_structure* parent;
				uint32_t offset = 0;

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

			void write_path(compilation_data& data, Command command, const std::string& path, container_structure* context, CommandFlag flags = CommandFlag::NO_FLAGS)
			{
				// Write command out with stub param
				write(data, command, (uint32_t)0, flags);;

				// Note to write over that param later
				size_t param_position = data.container_data.pos() - sizeof(uint32_t);
				data.paths.push_back(make_tuple(param_position, path, context));
			}

			void write_variable(compilation_data& data, Command command, const std::string& name)
			{
				// Hash the name of the variable
				uint32_t hash = hash_string(name.c_str());

				// Write it out
				write(data, command, hash);

				// TODO: Check for hash collisions?
			}

			void compile_command(const std::string& name, compilation_data& data);
			container_structure* compile_container(const nlohmann::json& container, compilation_data& data, container_structure* parent, int index_in_parent);
			void process_paths(compilation_data& data, container_structure* root);

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

				int index = -1;

				for (auto iter = container.begin(); iter != container.end(); ++iter)
				{
					index++;

					// Handle final object
					if (container.end() - 1 == iter)
					{
						// Should be an assert
						if (iter->is_object())
						{
							auto meta = *iter;
							for (auto& meta_iter : meta.items())
							{
								// Name
								if (meta_iter.key() == "#n")
								{
									// Add to parent's named children list
									if (parent != nullptr)
									{
										std::string name = meta_iter.value().get<std::string>();
										parent->named_children.insert({ name, self });
									}
								}
								// Flags
								else if (meta_iter.key() == "#f")
								{
									//std::cout << "Flags: " << meta_iter.value().get<int>() << std::endl;
									// TODO: Flags
								}
								// Child container
								else
								{
									// Add to deferred compilation list
									data.deferred.push(std::make_tuple(meta_iter.value(), meta_iter.key(), self));
								}
							}
						}
						continue;
					}

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

							// Is it a variable divert?
							auto isVar = iter->find("var");
							if (isVar != iter->end() && isVar->get<bool>())
							{
								write_variable(data, Command::DIVERT_TO_VARIABLE, path);
							}
							else
							{
								// Write path in DIVERT command
								write_path(data, Command::DIVERT, path, self);
							}
						}
						else if (iter->find("^->") != iter->end())
						{
							// Get the divert path
							auto path = (*iter)["^->"].get<std::string>();

							// Write path in DIVERT_VAL command
							write_path(data, Command::DIVERT_VAL, path, self);
						}
						// Temporary variable
						else if (iter->find("temp=") != iter->end())
						{
							// Get variable name
							auto name = (*iter)["temp="].get<std::string>();

							// Define temporary variable
							write_variable(data, Command::DEFINE_TEMP, name);
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
					}
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
					char* _context;
					const char* token = strtok_s(const_cast<char*>(path_cstr), ".", &_context);
					while (token != nullptr)
					{
						// Number
						if (std::isdigit(token[0]))
						{
							container = container->indexed_children[atoi(token)];
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

					// Write container address
					data.container_data.set(position, container->offset);
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

			// Deferred compilation
			while (data.deferred.size() > 0)
			{
				using std::get;

				// Defer compilation
				auto t = data.deferred.top();
				data.deferred.pop();

				// Add to named child list
				container_structure* namedChild = compile_container(get<0>(t), data, get<2>(t), -1);
				get<2>(t)->named_children.insert({ get<1>(t), namedChild });
			}

			// Handle path processing
			process_paths(data, root);

			// == Start writing to the file ==

			// Write the ink version
			out.write((const char*)&inkVersion, sizeof(int));

			// Write the string table
			data.string_table.write_to(out);

			// Write a separator
			out << (char)0;

			// Write the container data
			data.container_data.write_to(out);

			// Flush the file
			out.flush();

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