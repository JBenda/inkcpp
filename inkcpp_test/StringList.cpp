#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("run a story with just text", "[translation]")
{
	GIVEN("a story with just text")
	{
		inklecate("ink/StringListStory.ink", "StringListStory.tmp");
		ink::compiler::run("StringListStory.tmp", "StringListStory.bin", "StringListStory.bin.str");
		WHEN("use compiler output string list")
		{
			auto ink = story::create("StringListStory.bin", "StringListStory.bin.str");
			runner thread = ink->new_runner();
			THEN("print what is in StringListStory.ink")
			{
				REQUIRE(thread->getall() == "Just a short story with strings:\n"
						"a string in a variable\nand a another.\n");
			}
		}
		WHEN("use translated string list")
		{
			auto ink = story::create("StringListStory.bin", "ink/StringListStory_ger.str");
			runner thread = ink->new_runner();
			THEN("print translated story")
			{
				REQUIRE(thread->getall() == "Eine kleine Geschichte mit Zeichenketten:\n"
						"eine Zeichenkette in einer Variablen\nund noch eine.\n");
			}
		}
	}
}
