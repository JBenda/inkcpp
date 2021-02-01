#pragma once

#include "config.h"
#ifdef INK_EXPOSE_JSON
#include "../json.hpp"
#endif
#include "compilation_results.h"
#include <iostream>

namespace ink
{
	namespace compiler
	{
		// file -> file
		void run(const char* filenameIn, const char* filenameOut, compilation_results* results = nullptr);

		// file -> stream
		void run(const char* filenameIn, std::ostream& out, compilation_results* results = nullptr);

#ifdef INK_EXPOSE_JSON
		// JSON -> file
		void run(const nlohmann::json&, const char* filenameOut, compilation_results* results = nullptr);

		// JSON -> stream
		void run(const nlohmann::json&, std::ostream& out, compilation_results* results = nullptr);
#endif

		// stream -> stream
		void run(std::istream& in, std::ostream& out, compilation_results* results = nullptr);

		// stream -> file
		void run(std::istream& in, const char* filenameOut, compilation_results* results = nullptr);
	}
}
