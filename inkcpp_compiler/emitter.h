#pragma once

#include "command.h"
#include "system.h"
#include "reporter.h"
#include <string>
#include <vector>

namespace ink::compiler::internal
{
	class list_data;

	// Abstract base class for emitters which write ink commands to a file
	class emitter : public reporter
	{
	public:
		virtual ~emitter() { }

		// starts up the emitter (and calls initialize)
		void start(int ink_version, compilation_results* results = nullptr);

		// tells the emitter compilation is done (and calls finalize)
		void finish(container_t max_container_index);

		// start a container
		virtual uint32_t start_container(int index_in_parent, const std::string& name) = 0;

		// ends a container
		virtual uint32_t end_container() = 0;

		// checks if _root contains a container named name to check
		// if name is in valid internal function name
		// @return number of arguments functions takes (arity)
		// @retval -1 if the function was not found
		virtual int function_container_arguments(const std::string& name) = 0;

		// Writes a command with an optional payload
		virtual void write_raw(Command command, CommandFlag flag = CommandFlag::NO_FLAGS, const char* payload = nullptr, ink::size_t payload_size = 0) = 0;

		// Writes a command with a path as the payload
		virtual void write_path(Command command, CommandFlag flag, const std::string& path, bool useCountIndex = false) = 0;

		// Writes a command with a variable as the payload
		virtual void write_variable(Command command, CommandFlag flag, const std::string& name) = 0;

		// Writes a command with a string payload
		virtual void write_string(Command command, CommandFlag flag, const std::string& string) = 0;

		// write a command with a list payload
		virtual void write_list(Command commmand, CommandFlag flag, const std::vector<list_flag>& list) = 0;

		// Callback for nop commands
		virtual void handle_nop(int index_in_parent) = 0;

		// adds a fallthrough divert
		virtual uint32_t fallthrough_divert() = 0;

		// Patches a fallthrough divert at the given position to divert to the current position
		virtual void patch_fallthroughs(uint32_t position) = 0;

		// Adds a container start marker to the container map
		void add_start_to_container_map(uint32_t offset, container_t index);

		// Adds a container end marker to the container map
		void add_end_to_container_map(uint32_t offset, container_t index);

		// add list definitions
		virtual void set_list_meta(const list_data& lists_defs) = 0;

		// Helpers
		template<typename T>
		void write(Command command, const T& param, CommandFlag flag = CommandFlag::NO_FLAGS)
		{
			static_assert(sizeof(T) == 4, "Parameters must be 4 bytes long");
			write_raw(command, flag, (const char*)(&param), sizeof(T));
		}

	protected:
		// Initialize (clear state, get ready for a new file)
		virtual void initialize() = 0;

		// Finalize (do any post processing necessary)
		virtual void finalize() = 0;

		// Set container index for visit tracking
		virtual void setContainerIndex(container_t index) = 0;

	protected:
		typedef std::vector<std::pair<uint32_t, container_t>> container_map;

		// container map
		container_map _container_map;
		container_t _max_container_index;

		// ink version
		int _ink_version;
	};
}
