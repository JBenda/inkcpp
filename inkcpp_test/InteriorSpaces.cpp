#include "catch.hpp"

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
} // namespace

SCENARIO("a run of spaces inside a line survives to the output", "[regression][runtime]")
{
	GIVEN("a story whose text contains four consecutive spaces")
	{
		std::unique_ptr<story> ink{
		    compile_json(R"({"inkVersion":21,"root":[["^A    B","\n","done",null],"done",{"#f":1}],)"
		                 R"("listDefs":{}})")
		};
		runner main = ink->new_runner();

		WHEN("the line is read")
		{
			THEN("the spaces are still there") { REQUIRE(chomp(main->getline()) == "A    B"); }
		}
	}
}

SCENARIO("spaces that meet at a seam still collapse", "[regression][runtime]")
{
	GIVEN("two glued fragments, one ending and one beginning with a space")
	{
		std::unique_ptr<story> ink{compile_json(
		    R"({"inkVersion":21,"root":[["^Knock ","<>","^ again?","\n","done",null],"done",)"
		    R"({"#f":1}],"listDefs":{}})"
		)};
		runner                 main = ink->new_runner();

		WHEN("the line is read")
		{
			THEN("the join reads as one space") { REQUIRE(chomp(main->getline()) == "Knock again?"); }
		}
	}
}
