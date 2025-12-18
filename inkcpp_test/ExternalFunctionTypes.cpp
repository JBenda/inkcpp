#include "catch.hpp"

#include <../runner_impl.h>
#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story with external functions support types", "[story]")
{
	GIVEN("a story with external functions")
	{
		auto ink    = story::from_file(INK_TEST_RESOURCE_DIR "ExternalFunctionTypes.bin");
		auto thread = ink->new_runner().cast<internal::runner_impl>();

		std::stringstream debug;
		thread->set_debug_enabled(&debug);

		bool         b = false;
		int          i = 0;
		unsigned int u = 0;
		float        f = 0;
		std::string  s;

		thread->bind("SET_BOOL", [&b](bool o) { b = o; });
		thread->bind("SET_INT", [&i](int o) { i = o; });
		thread->bind("SET_UINT", [&u](unsigned int o) { u = o; });
		thread->bind("SET_FLOAT", [&f](float o) { f = o; });
		thread->bind("SET_STRING", [&s](std::string o) { s = o; });

		thread->bind("GET_BOOL", [&b]() { return b; });
		thread->bind("GET_INT", [&i]() { return i; });
		thread->bind("GET_UINT", [&u]() { return u; });
		thread->bind("GET_FLOAT", [&f]() { return f; });
		thread->bind("GET_STRING", [&s]() { return s; });

		WHEN("run thread")
		{
			THEN("thread has correct line counts")
			{
				auto line = thread->getline();
				REQUIRE(line == "true 1.5 -5 17 foo\n");
			}
		}
	}
}
