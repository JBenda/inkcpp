#pragma once

#include "config.h"
#ifdef INK_EXPOSE_JSON
#include "json.hpp"
#endif
#include <iostream>

namespace ink
{
	namespace compiler
	{
		// file -> file
		void run(const char* filenameIn, const char* filenameOut);

		// file -> stream
		void run(const char* filenameIn, std::ostream& out);

#ifdef INK_EXPOSE_JSON
		// JSON -> file
		void run(const nlohmann::json&, const char* filenameOut);

		// JSON -> stream
		void run(const nlohmann::json&, std::ostream& out);
#endif

		// stream -> stream
		void run(std::istream& in, std::ostream& out);

		// stream -> file
		void run(std::istream& in, const char* filenameOut);
	}
}