#pragma once

#include "command.h"
#include "system.h"
#include <string>
#include <vector>

namespace ink::compiler
{
	struct compilation_results;
}

namespace ink::compiler::internal
{
	typedef std::vector<std::pair<uint32_t, container_t>> container_map;

	// Abstract base class for emitters which write ink commands to a file
	class emitter
	{
	public:
		// start a container
		virtual uint32_t start_container(int index_in_parent, const std::string& name) = 0;

		// ends a container
		virtual uint32_t end_container() = 0;

		// Writes a command with an optional payload
		virtual void write_raw(Command command, CommandFlag flag = CommandFlag::NO_FLAGS, const char* payload = nullptr, ink::size_t payload_size = 0) = 0;

		// Writes a command with a path as the payload
		virtual void write_path(Command command, CommandFlag flag, const std::string& path, bool useCountIndex = false) = 0;

		// Writes a command with a variable as the payload
		virtual void write_variable(Command command, CommandFlag flag, const std::string& name) = 0;

		// Writes a command with a string payload
		virtual void write_string(Command command, CommandFlag flag, const std::string& string) = 0;

		// Callback for nop commands
		virtual void handle_nop(int index_in_parent) = 0;
		
		// Finalize (do any post processing and make sure file is emitted in full)
		virtual void finalize(const container_map&, container_t) = 0;

		// Initialize (clear state, get ready for a new file)
		virtual void initialize(int inkVersion, compilation_results*) = 0;

		// adds a fallthrough divert
		virtual uint32_t fallthrough_divert() = 0;

		// Patches a fallthrough divert at the given position to divert to the current position
		virtual void patch_fallthroughs(uint32_t position) = 0;

		// Helpers
		template<typename T>
		void write(Command command, const T& param, CommandFlag flag = CommandFlag::NO_FLAGS)
		{
			static_assert(sizeof(T) == 4, "Parameters must be 4 bytes long");
			write_raw(command, flag, (const char*)(&param), sizeof(T));
		}
	};
}