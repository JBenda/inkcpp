#include "catch.hpp"
#include "system.h"

#include <choice.h>
#include <compiler.h>
#include <runner.h>
#include <story.h>
#include <globals.h>

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
	return story::from_binary(copy, static_cast<ink::size_t>(bytes.size()), true);
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
    R"("^before end \t\t","\n",)"
    R"("ev","str","^Pick   me","/str","/ev",{"*":"0.c-0","flg":20},)"
    R"({"c-0":["\n","end",{"->":"0.g-0"},{"#f":5}],"g-0":["done",null]}],"done",null],)"
    R"("listDefs":{}})";
// clang-format on

/** everything the runner emits for the story above */
struct transcript {
	std::string spaces;
	std::string tabs;
	std::string space_seam;
	std::string tab_seam;
	std::string trailing_run;
	std::string choice;
};

transcript read_all(runner& main)
{
	transcript out;
	out.spaces       = chomp(main->getline());
	out.tabs         = chomp(main->getline());
	out.space_seam   = chomp(main->getline());
	out.tab_seam     = chomp(main->getline());
	out.trailing_run = main->getline();
	REQUIRE(main->num_choices() == 1);
	out.choice = main->get_choice(0)->text();
	return out;
}
} // namespace

SCENARIO("runs of whitespace inside a line", "[runtime][output]")
{
	GIVEN("a story with runs of spaces and tabs")
	{
		std::unique_ptr<story> ink{compile_json(story_json)};

		WHEN("no whitespace mode is set")
		{
			runner     main = ink->new_runner();
			transcript out  = read_all(main);

			THEN("each run collapses to one character, as the reference ink runtime does")
			{
				REQUIRE(out.spaces == "A B");
				REQUIRE(out.tabs == "tab\tsep");
				REQUIRE(out.choice == "Pick me");
			}
			THEN("whitespace where two fragments are glued reads as one")
			{
				REQUIRE(out.space_seam == "Knock again?");
				REQUIRE(out.tab_seam == "Knock again?");
			}
			THEN("whitespace at the end of a line is dropped")
			{
				REQUIRE(out.trailing_run == "before end\n");
			}
		}

		WHEN("the whitespace mode is keep_runs")
		{
			runner main = ink->new_runner();
			main->set_whitespace_mode(whitespace_mode::keep_runs);
			transcript out = read_all(main);

			THEN("runs inside a line are kept, in lines and in choice text")
			{
				REQUIRE(out.spaces == "A    B");
				REQUIRE(out.tabs == "tab\t\tsep");
				REQUIRE(out.choice == "Pick   me");
			}
			THEN("whitespace where two fragments are glued still reads as one")
			{
				REQUIRE(out.space_seam == "Knock again?");
				REQUIRE(out.tab_seam == "Knock\tagain?");
			}
			THEN("a run at the end of a line is still dropped")
			{
				REQUIRE(out.trailing_run == "before end\n");
			}
		}

		WHEN("the whitespace mode is set back to collapse")
		{
			runner main = ink->new_runner();
			main->set_whitespace_mode(whitespace_mode::keep_runs);
			main->set_whitespace_mode(whitespace_mode::collapse);
			transcript out = read_all(main);

			THEN("the runs collapse again") { REQUIRE(out.spaces == "A B"); }
		}
	}
}
