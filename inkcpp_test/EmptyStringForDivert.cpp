#include "catch.hpp"
#include "../inkcpp_cl/test.h"

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
		inklecate("ink/EmptyStringForDivert.ink", "EmptyStringForDivert.tmp");
		ink::compiler::run("EmptyStringForDivert.tmp", "EmptyStringForDivert.bin");
		auto   ink    = story::from_file("EmptyStringForDivert.bin");
		runner thread = ink->new_runner();

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
