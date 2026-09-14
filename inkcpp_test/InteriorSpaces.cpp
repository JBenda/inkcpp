#include "catch.hpp"

#include <choice.h>
#include <compiler.h>
#include <runner.h>
#include <story.h>

#include <cstring>
#include <memory>
#include <sstream>
#include <string>

using namespace ink::runtime;

namespace
{
story* compile_json(const char* json)
{
	std::istringstream in{json};
	std::stringstream  out;
	ink::compiler::run(in, out);
	const std::string bytes = out.str();

	auto* copy = new unsigned char[bytes.size()];
	std::memcpy(copy, bytes.data(), bytes.size());
	return story::from_binary(copy, bytes.size(), true);
}

std::string chomp(std::string line)
{
	while (! line.empty() && (line.back() == '\n' || line.back() == '\r')) {
		line.pop_back();
	}
	return line;
}

// clang-format off
const char* const story_json =
    R"({"inkVersion":21,"root":[[)"
    R"("^A    B","\n",)"
    R"("^tab\t\tsep","\n",)"
    R"("^Knock ","<>","^ again?","\n",)"
    R"("^Knock\t","<>","^ again?","\n",)"
    R"("^before end   ","\n",)"
    R"("ev","str","^Pick   me","/str","/ev",{"*":"0.c-0","flg":20},)"
    R"({"c-0":["\n","end",{"->":"0.g-0"},{"#f":5}],"g-0":["done",null]}],"done",null],)"
    R"("listDefs":{}})";
// clang-format on
} // namespace

SCENARIO("runs of whitespace inside a line", "[runtime][output]")
{
	GIVEN("a story with runs of spaces and tabs")
	{
		std::unique_ptr<story> ink{compile_json(story_json)};
		runner                 main = ink->new_runner();

		std::string spaces       = chomp(main->getline());
		std::string tabs         = chomp(main->getline());
		std::string space_seam   = chomp(main->getline());
		std::string tab_seam     = chomp(main->getline());
		std::string trailing_run = main->getline();
		REQUIRE(main->num_choices() == 1);
		std::string choice = main->get_choice(0)->text();

		THEN("spaces meeting where two fragments are glued read as one")
		{
			REQUIRE(space_seam == "Knock again?");
		}
		THEN("whitespace at the end of a line is dropped") { REQUIRE(trailing_run == "before end\n"); }
#ifdef INKCPP_KEEP_SPACE_RUNS
		THEN("with INKCPP_KEEP_SPACE_RUNS the runs are kept")
		{
			REQUIRE(spaces == "A    B");
			REQUIRE(tabs == "tab\t\tsep");
			REQUIRE(tab_seam == "Knock\tagain?");
			REQUIRE(choice == "Pick   me");
		}
#else
		THEN("by default each run collapses to one character, like the reference ink runtime")
		{
			REQUIRE(spaces == "A B");
			REQUIRE(tabs == "tab\tsep");
			REQUIRE(tab_seam == "Knock again?");
			REQUIRE(choice == "Pick me");
		}
#endif
	}
}
