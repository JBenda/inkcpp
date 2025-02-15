#include "catch.hpp"
#include "system.h"

#include <story.h>
#include <globals.h>
#include <choice.h>
#include <runner.h>

#include <iostream>

using namespace ink::runtime;

SCENARIO("string_table fill up #97", "[fixes]")
{
	GIVEN("story murder_scene")
	{
		auto    ink       = story::from_file(INK_TEST_RESOURCE_DIR "murder_scene.bin");
		globals globStore = ink->new_globals();
		runner  main      = ink->new_runner(globStore);

		WHEN("Run first choice 50 times")
		{
			std::string story = "";
			main->getall();
			while (main->has_choices()) {
				main->choose(0);
				story += main->getall();
			}
			THEN("string table should still have room")
			{
				REQUIRE(story.length() == 3082);
				// TEST string table size
			}
		}
	}
}
