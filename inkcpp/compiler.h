#pragma once

#include "json.hpp"
#include <iostream>

namespace ink
{
	namespace compiler
	{
		// file -> file
		void run(const char* filenameIn, const char* filenameOut);

		// file -> stream
		void run(const char* filenameIn, std::ostream& out);

		// JSON -> file
		void run(const nlohmann::json&, const char* filenameOut);

		// JSON -> stream
		void run(const nlohmann::json&, std::ostream& out);

		// stream -> stream
		void run(std::istream& in, std::ostream& out);

		// stream -> file
		void run(std::istream& in, const char* filenameOut);
	}
}