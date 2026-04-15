#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story with a white space infront of an conditional Divert", "[Output]")
{
	// based on https://github.com/JBenda/inkcpp/issues/71
	GIVEN("A story")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "EmptyStringForDivert.bin")};
		runner                 thread = ink->new_runner();

		WHEN("run")
		{
			THEN("print 'This displays first'")
			{
				thread->getall();
				REQUIRE(thread->has_choices());
				thread->choose(0);
				REQUIRE(thread->getall() == "This displays first\n");
				REQUIRE(thread->has_choices());
				thread->choose(0);
				REQUIRE(thread->getall() == "This is the continuation.\n");
				REQUIRE(thread->has_choices());
				thread->choose(0);
				REQUIRE(thread->getall() == "");
				REQUIRE(! thread->has_choices());
			}
		}
	}
}
