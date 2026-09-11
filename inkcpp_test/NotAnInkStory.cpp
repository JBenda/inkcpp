#include "catch.hpp"

#include <compiler.h>
#include <system.h>

#include <sstream>
#include <string>

/* Anything at all can be handed to a compiler that accepts "a .json file": a
 * settings file, an API response, a package.json. Before the guards in
 * json_compiler::compile() and compile_container(), reaching for the top-level
 * keys with operator[] made that fatal rather than reportable - on a const
 * nlohmann::json a missing key is an assert(), so the process aborted with
 * assertions on and dereferenced an end iterator with them off.
 *
 * These cases are the ones that were measured to abort, segfault, or silently
 * emit a bogus binary. They are all cheap to state, and the failure they guard
 * against is the kind that only shows up in someone else's crash report. */

using namespace ink::compiler;

namespace
{
std::string compile_or_throw(const char* json)
{
	std::istringstream in(json);
	std::ostringstream out;
	run(in, out);
	return out.str();
}
} // namespace

SCENARIO("JSON that is not an Ink story is reported, not fatal", "[compiler]")
{
	GIVEN("a document whose top level is not an object")
	{
		auto source = GENERATE("[1, 2, 3]", "\"just a string\"", "42", "null");
		THEN("compiling it raises rather than aborting")
		{
			REQUIRE_THROWS_AS(compile_or_throw(source), ink::ink_exception);
		}
	}

	GIVEN("an object without the keys a story must have")
	{
		// Previously an assert() on the missing key, so an abort() with
		// assertions enabled.
		auto source = GENERATE(
		    "{}",                                        // neither key
		    "{\"inkVersion\": 21}",                      // no root
		    "{\"root\": [[\"done\"], null]}",            // no inkVersion
		    "{\"name\": \"not a story\", \"value\": 42}" // something else entirely
		);
		THEN("compiling it raises rather than aborting")
		{
			REQUIRE_THROWS_AS(compile_or_throw(source), ink::ink_exception);
		}
	}

	GIVEN("a root that is not a non-empty array")
	{
		// compile_container() takes rbegin() and end() - 1, which are only
		// meaningful for a non-empty array. An empty array or an object used to
		// dereference an end iterator; a scalar root silently produced a small
		// binary that was not a story.
		auto source = GENERATE(
		    "{\"inkVersion\": 21, \"root\": []}", "{\"inkVersion\": 21, \"root\": {}}",
		    "{\"inkVersion\": 21, \"root\": \"nope\"}", "{\"inkVersion\": 21, \"root\": 42}"
		);
		THEN("compiling it raises rather than aborting")
		{
			REQUIRE_THROWS_AS(compile_or_throw(source), ink::ink_exception);
		}
	}

	GIVEN("a minimal but valid story")
	{
		THEN("it still compiles, so the guards have not become too strict")
		{
			std::string binary;
			REQUIRE_NOTHROW(
			    binary = compile_or_throw("{\"inkVersion\": 21, \"root\": [[\"done\"], null], "
			                              "\"listDefs\": {}}")
			);
			REQUIRE(binary.size() > 0);
		}
	}
}
