#include "json_compiler.h"

#include "command.h"
#include "list_data.h"
#include "system.h"
#include "version.h"

#include <string_view>

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
		_ink_version = input["inkVersion"];

		// Start the output
		set_results(results);
		_emitter = output;

		// Initialize emitter
		_emitter->start(_ink_version, results);

		if(auto itr = input.find("listDefs"); itr != input.end()) {
			compile_lists_definition(*itr);
			_emitter->set_list_meta(_list_meta);
		}
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
		CommandFlag cmd_flags = CommandFlag::NO_FLAGS;
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
						data.cmd_flags = CommandFlag::NO_FLAGS;
						if (visits)
							data.cmd_flags |= CommandFlag::CONTAINER_MARKER_TRACK_VISITS;
						if (turns)
							data.cmd_flags |= CommandFlag::CONTAINER_MARKER_TRACK_TURNS;
						if (onlyFirst)
							data.cmd_flags |= CommandFlag::CONTAINER_MARKER_ONLY_FIRST;


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
		// Write command out at this position
		if(meta.cmd_flags != CommandFlag::NO_FLAGS) {
			_emitter->write(Command::START_CONTAINER_MARKER, meta.indexToReturn, meta.cmd_flags);
		}
		if(meta.recordInContainerMap) {
			_emitter->add_start_to_container_map(position, meta.indexToReturn);
		}

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

			// Booleans
			else if (iter->is_boolean())
			{
				int value = iter->get<bool>() ? 1 : 0;
				_emitter->write(Command::BOOL, value);
			}

			// Complex commands
			else if (iter->is_object())
			{
				compile_complex_command(*iter);
			}

			else {
				throw ink_exception("Failed to container member!");
			}
		}

		if (meta.deferred.size() > 0)
		{
			std::vector<size_t> divert_positions;

			// Write empty divert to be patched later
			uint32_t divert_position = _emitter->fallthrough_divert();
			divert_positions.push_back(divert_position);

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

		// End container
		uint32_t end_position = _emitter->end_container();

		// Write end container marker, End pointer should point to End command (form symetry with START command)
		if (meta.indexToReturn != ~0)
			_emitter->write(Command::END_CONTAINER_MARKER, meta.indexToReturn, meta.cmd_flags);

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
			bool is_var;
			if(get(command, "var", is_var) && is_var) {
				_emitter->write_variable(Command::TUNNEL,
						CommandFlag::TUNNEL_TO_VARIABLE,
						val);
			} else {
				_emitter->write_path(Command::TUNNEL, CommandFlag::NO_FLAGS, val);
			}
		}

		// Declare temporary variable
		else if (get(command, "temp=", val))
		{
			bool is_redef = false;
			get(command, "re", is_redef);
			_emitter->write_variable(Command::DEFINE_TEMP,
					is_redef ? CommandFlag::ASSIGNMENT_IS_REDEFINE : CommandFlag::NO_FLAGS,
					val);
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

		// create pointer value
		else if (get(command, "^var", val)) {
			int ci;
			if(!get(command, "ci", ci)) { throw ink_exception("failed to parse ci for pointer!");}
			inkAssert(ci < 255, "only support until 255 stack hight for refernces");
			_emitter->write_variable(Command::VALUE_POINTER, static_cast<CommandFlag>(ci+1), val);
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
			bool is_var; // function address is stored in jump
			if(get(command, "var", is_var) && is_var) {
				_emitter->write_variable(Command::FUNCTION,
						CommandFlag::FUNCTION_TO_VARIABLE,
						val);
			} else {
				_emitter->write_path(Command::FUNCTION, CommandFlag::NO_FLAGS, val);
			}
		}

		// External function call
		else if (get(command, "x()", val))
		{
			// Get argument count
			int numArgs = 0;
			get(command, "exArgs", numArgs);

			// Encode argument count into command flag and write out the hash of the function name
			_emitter->write(Command::CALL_EXTERNAL, hash_string(val.c_str()),
					static_cast<CommandFlag>(numArgs));
			_emitter->write_path(Command::FUNCTION, CommandFlag::FALLBACK_FUNCTION, val);
	}

		// list initialisation
		else if (has(command, "list"))
		{
			std::vector<list_flag> entries;
			auto& list = command["list"];

			if(list.size()) {
				for ( const auto& [key,value] : list.items()) {
					entries.push_back({
							_list_meta.get_lid(key.substr(0,key.find('.'))),
							static_cast<decltype(list_flag::flag)>(value.get<int>() - 1)
					});

				}
			} else {
				if(has(command, "origins")) {
					for( const auto& origin_list : command["origins"]) {
						entries.push_back({ _list_meta.get_lid(origin_list.get<std::string>()), -1 });
					}
				} else {
					entries.push_back(empty_flag);
				}
			}

			_emitter->write_list(Command::LIST, CommandFlag::NO_FLAGS, entries);
		}

		else if (get(command, "#", val))
		{
			if (_ink_version > 20) {
				ink_exception("with inkVerison 21 the tag system chages, and the '#: <tag>' is deprecated now");
			}
			_emitter->write_string(Command::TAG, CommandFlag::NO_FLAGS, val);
		}

		else {
			throw ink_exception("failed to parse complex command!");
		}
	}

	void json_compiler::compile_lists_definition(const nlohmann::json& list_defs)
	{
		for(auto& [list_name, flags] : list_defs.items())	{
			_list_meta.new_list(list_name);
			for(auto& [flag_name, value] : flags.items()) {
				_list_meta.new_flag(flag_name, value.get<int>());
			}
		}
	}
}
