#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <runner.h>
#include <compiler.h>

#include <sstream>
#include <fstream>

using namespace ink::runtime;

SCENARIO("a story supports UTF-8", "[utf-8]")
{
	GIVEN("a story with UTF8 characters")
	{
		inklecate("ink/UTF8Story.ink", "UTF8Story.tmp");
		ink::compiler::run("UTF8Story.tmp", "UTF8Story.bin");
		auto ink = story::from_file("UTF8Story.bin");
		runner thread = ink->new_runner();

		std::ifstream demoFile("ink/UTF-8-demo.txt");
		if (!demoFile.is_open()) {
			throw std::runtime_error("cannot open UTF-8 demo file");
		}

		char byte;
		std::stringstream demo;
		std::stringstream current;
		while (demoFile.get(byte)) {
			if (byte == '\r') continue; // skip windows carriage-return newlines
			else if (byte == '\n' || byte == 0) { // newline or null byte
				std::string s = current.str();
				if (s.empty()) continue;
				demo << s << '\n';
				current.str("");
			}
			else { // normal char
				current << byte;
			}
		}
		std::string s = current.str();
		if (!s.empty()) demo << s << '\n';
		

		WHEN("consume lines") {
			std::stringstream content;
			while (thread->can_continue()) {
				content << thread->getline();
			}
			THEN("lines match test file") {
				std::string c = content.str();
				std::string d = demo.str();
				REQUIRE(c == d);
			}
		}
	}
}
