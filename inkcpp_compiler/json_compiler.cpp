#include "json_compiler.h"
#include <iostream>

namespace ink::compiler::internal
{
	using nlohmann::json;
	using std::vector;

	typedef std::tuple<json, std::string> defer_entry;

	json_compiler::json_compiler()
		: _emitter(nullptr), _next_container_index(0)
	{ }

	void json_compiler::compile(const nlohmann::json& input, emitter* output, compilation_results* results)
	{
		// Get the runtime version
		int inkVersion = input["inkVersion"];
		// TODO: Do something with version number

		// Start the output
		set_results(results);
		_emitter = output;

		// Initialize emitter
		_emitter->start(inkVersion, results);

		// Compile the root container
		compile_container(input["root"], 0);

		// finalize
		_emitter->finish(_next_container_index);

		// Clear
		_emitter = nullptr;
		_next_container_index = 0;
		clear_results();
	}

	struct container_meta
	{
		std::string name;
		container_t indexToReturn = ~0;
		bool recordInContainerMap = false;
		vector<defer_entry> deferred;
	};

	void json_compiler::handle_container_metadata(
		const json& meta, container_meta& data)
	{
		if (meta.is_object())
		{
			for (auto& meta_iter : meta.items())
			{
				// Name
				if (meta_iter.key() == "#n")
				{
					data.name = meta_iter.value().get<std::string>();
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
						container_t myIndex = _next_container_index++;

						// Make appropriate flags
						CommandFlag flags = CommandFlag::NO_FLAGS;
						if (visits)
							flags |= CommandFlag::CONTAINER_MARKER_TRACK_VISITS;
						if (turns)
							flags |= CommandFlag::CONTAINER_MARKER_TRACK_TURNS;

						// Write command out at this position
						_emitter->write(Command::START_CONTAINER_MARKER, myIndex, flags);

						data.indexToReturn = myIndex;

						//if (!onlyFirst) // ????
						{
							data.recordInContainerMap = true;
						}
					}
				}
				// Child container
				else
				{
					// Add to deferred compilation list
					data.deferred.push_back(std::make_tuple(meta_iter.value(), meta_iter.key()));
				}
			}
		}
	}

	void json_compiler::compile_container(
		const nlohmann::json& container, int index_in_parent,
		const std::string& name_override)
	{
		// Grab metadata from the last object in this container
		container_meta meta;
		handle_container_metadata(*container.rbegin(), meta);

		// tell the emitter we're beginning a new container
		uint32_t position = _emitter->start_container(index_in_parent, name_override.empty() ? meta.name : name_override);
		if(meta.recordInContainerMap)
			_emitter->add_start_to_container_map(position, meta.indexToReturn);

		// Now, we want to iterate children of this container, save the last
		//  The last is the settings object handled above
		int index = -1;
		auto end = container.end() - 1;
		for (auto iter = container.begin(); iter != end; ++iter)
		{
			// Increment index
			index++;

			// Arrays are child containers. Recurse.
			if (iter->is_array())
				compile_container(*iter, index);

			// Strings are either commands, nops, or raw strings
			else if (iter->is_string())
			{
				// Get the string
				std::string string = iter->get<std::string>();

				if (string[0] == '^')
					_emitter->write_string(Command::STR, CommandFlag::NO_FLAGS, string);
				else if (string == "nop")
					_emitter->handle_nop(index);
				else
					compile_command(string);
			}

			// Numbers (floats and integers)
			else if (iter->is_number())
			{
				if (iter->is_number_float())
				{
					float value = iter->get<float>();
					_emitter->write(Command::FLOAT, value);
				}
				else
				{
					int value = iter->get<int>();
					_emitter->write(Command::INT, value);
				}
			}

			// Complex commands
			else if (iter->is_object())
			{
				compile_complex_command(*iter);
			}
		}

		if (meta.deferred.size() > 0)
		{
			std::vector<size_t> divert_positions;

			// Write empty divert to be patched later
			uint32_t position = _emitter->fallthrough_divert();
			divert_positions.push_back(position);

			// (2) Write deffered containers
			for (auto& t : meta.deferred)
			{
				using std::get;

				// Add to named child list
				compile_container(get<0>(t), -1, get<1>(t));

				// Need a divert here
				uint32_t pos = _emitter->fallthrough_divert();
				divert_positions.push_back(pos);
			}

			// (3) Set divert positions
			for (size_t offset : divert_positions)
				_emitter->patch_fallthroughs(offset);
		}

		// Write end container marker
		if (meta.indexToReturn != ~0)
			_emitter->write(Command::END_CONTAINER_MARKER, meta.indexToReturn);

		// End container
		uint32_t end_position = _emitter->end_container();

		// Record end position in map
		if (meta.recordInContainerMap)
			_emitter->add_end_to_container_map(end_position, meta.indexToReturn);
	}

	void json_compiler::compile_command(const std::string& command)
	{
		// Find command
		for (int i = 0; i < (int)Command::NUM_COMMANDS; i++)
		{
			if (CommandStrings[i] != nullptr && command == CommandStrings[i])
			{
				_emitter->write_raw((Command)i, CommandFlag::NO_FLAGS, nullptr, 0);
				return;
			}
		}

		// Missing command warning
		err() << "Unknown command '" << command << "'. Skipping." << std::flush;
	}

	void json_compiler::compile_complex_command(const nlohmann::json& command)
	{
		std::string val;

		// Divert
		if (get(command, "->", val))
		{
			// Check if this is a conditional divert
			bool isConditional = false;
			CommandFlag flag = CommandFlag::NO_FLAGS;
			if (get(command, "c", isConditional) && isConditional)
				flag = CommandFlag::DIVERT_HAS_CONDITION;

			// Switch on whether this is a variable divert or a path divert
			bool isVariableDivert = false;
			if (get(command, "var", isVariableDivert) && isVariableDivert)
				_emitter->write_variable(Command::DIVERT_TO_VARIABLE, flag, val);
			else
				_emitter->write_path(Command::DIVERT, flag, val);
		}

		// Divert to a value
		else if (get(command, "^->", val))
		{
			// Write path in a divert_val command
			_emitter->write_path(Command::DIVERT_VAL, CommandFlag::NO_FLAGS, val);
		}

		// Tunnel
		else if (get(command, "->t->", val))
		{
			_emitter->write_path(Command::TUNNEL, CommandFlag::NO_FLAGS, val);
		}

		// Declare temporary variable
		else if (get(command, "temp=", val))
		{
			_emitter->write_variable(Command::DEFINE_TEMP, CommandFlag::NO_FLAGS, val);
		}

		// Set variable
		else if (get(command, "VAR=", val))
		{
			// check if it's a redefinition
			bool is_redef = false;
			get(command, "re", is_redef);

			// Set variable
			_emitter->write_variable(Command::SET_VARIABLE,
				is_redef ? CommandFlag::ASSIGNMENT_IS_REDEFINE : CommandFlag::NO_FLAGS,
				val);
		}

		// Push variable
		else if (get(command, "VAR?", val))
		{
			_emitter->write_variable(Command::PUSH_VARIABLE_VALUE, CommandFlag::NO_FLAGS, val);
		}

		// Choice
		else if (get(command, "*", val))
		{
			// Get flags
			int flags = 0;
			get(command, "flg", flags);

			// Write choice path
			_emitter->write_path(Command::CHOICE, (CommandFlag)flags, val);
		}

		// Read count
		else if (get(command, "CNT?", val))
		{
			// TODO: Why is this true again?
			_emitter->write_path(Command::READ_COUNT, CommandFlag::NO_FLAGS, val, true);
		}

		// Internal function call
		else if (get(command, "f()", val))
		{
			_emitter->write_path(Command::FUNCTION, CommandFlag::NO_FLAGS, val);
		}

		// External function call
		else if (get(command, "x()", val))
		{
			// Get argument count
			int numArgs = 0;
			get(command, "exArgs", numArgs);

			// Encode argument count into command flag and write out the hash of the function name
			_emitter->write(Command::CALL_EXTERNAL, hash_string(val.c_str()), (CommandFlag)numArgs);
		}

		// list initialisation
		else if (has(command, "list"))
		{
			for ( const auto& entry : command["list"]) {
				
			}
		}
	}
}
