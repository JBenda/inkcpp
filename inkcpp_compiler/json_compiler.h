#pragma once
#include "json.hpp"
#include "system.h"
#include "compilation_results.h"
#include "emitter.h"
#include "reporter.h"
#include "list_data.h"

#include <vector>

namespace ink::compiler::internal
{
	struct container_meta;

	// Compiles ink json and outputs using a given emitter
	class json_compiler : public reporter
	{
	public:
		// create new compiler
		json_compiler();

		// compile from json using an emitter
		void compile(const nlohmann::json& input, emitter* output, compilation_results* results = nullptr);

	private: // == Compiler methods ==
		void handle_container_metadata(const nlohmann::json& meta, container_meta& data);
		void compile_container(const nlohmann::json& container, int index_in_parent, const std::string& name_override = "");
		void compile_command(const std::string& command);
		void compile_complex_command(const nlohmann::json& command);
		void compile_lists_definition(const nlohmann::json& list_defs);

	private: // == JSON Helpers ==
		inline bool has(const nlohmann::json& json, const std::string& key)
		{
			return json.find(key) != json.end();
		}

		template<typename T>
		bool get(const nlohmann::json& json, const std::string& key, T& value)
		{
			auto iter = json.find(key);
			if (iter == json.end())
				return false;

			value = iter->get<T>();
			return true;
		}

	private: // == Private members ==
		emitter* _emitter;
		container_t _next_container_index;

		list_data _list_meta;
		int _ink_version;
	};
}
