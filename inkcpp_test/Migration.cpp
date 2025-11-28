#include "catch.hpp"
#include "snapshot.h"
#include "../snapshot_impl.h"

#include <story.h>
#include <globals.h>
#include <choice.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("Introduce new glbal variables")
{
	std::unique_ptr<story> base_story{story::from_file(INK_TEST_RESOURCE_DIR "MigrationBase.bin")};
	globals                base_globals = base_story->new_globals();
	runner                 base_thread  = base_story->new_runner(base_globals);
	WHEN("Just Run the base story")
	{
		std::string content = base_thread->getall();
		REQUIRE(base_thread->has_choices());
		REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
		THEN("All values as defined")
		{
			CHECK(base_globals->get<int32_t>("do_not_migrate").value_or(0) == 10);
			CHECK(base_globals->get<int32_t>("do_migrate").value_or(0) == 15);
		}
	}
	GIVEN("Simple story with changes in globals.")
	{
		std::unique_ptr<story> new_story{story::from_file(INK_TEST_RESOURCE_DIR
		                                                  "MigrationChangeGlobals.bin")};
		WHEN("Just Run the new story")
		{
			globals     new_globals = new_story->new_globals();
			runner      new_thread  = new_story->new_runner(new_globals);
			std::string content     = new_thread->getall();
			REQUIRE(new_thread->has_choices());
			REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
			THEN("All values as defined")
			{
				CHECK(new_globals->get<int32_t>("do_migrate").value_or(0) == 10);
				CHECK(new_globals->get<int32_t>("new_var").value_or(0) == 20);
			}
		}
		WHEN("Run base story and load in new_story")
		{
			std::string content = base_thread->getall();
			REQUIRE(base_thread->has_choices());
			REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
			base_thread->choose(0);
			std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
			REQUIRE(snap->can_be_migrated());
			globals new_globals = new_story->new_globals_from_snapshot(*snap);
			runner  new_thread  = new_story->new_runner_from_snapshot(*snap, new_globals);
			THEN("expect merged globals")
			{
				CHECK_FALSE(new_globals->get<int32_t>("do_not_migrate").has_value());
				CHECK(new_globals->get<int32_t>("do_migrate").value_or(0) == 15);
				CHECK(new_globals->get<int32_t>("new_var").value_or(0) == 20);
			}
			THEN("expect story to continue normally")
			{
				content = new_thread->getall();
				REQUIRE(content == "A\ncatch\n1 -1 0\nOh.\n");
			}
		}
	}
	GIVEN("Simple story with changed knots.")
	{
		std::unique_ptr<story> new_story{story::from_file(INK_TEST_RESOURCE_DIR
		                                                  "MigrationChangeNodes.bin")};
		WHEN("Just Run the new story")
		{
			globals     new_globals = new_story->new_globals();
			runner      new_thread  = new_story->new_runner(new_globals);
			std::string content     = new_thread->getall();
			REQUIRE(new_thread->has_choices());
			REQUIRE(content == "B\n-1 0 0\nThis is a simple story.\n");
			new_thread->choose(0);
			content = new_thread->getall();
			REQUIRE(content == "A\ncatch\n-1 1 0\nOh.\n");
		}
		WHEN("Run base story and load new story.")
		{
			std::string content = base_thread->getall();
			REQUIRE(base_thread->has_choices());
			REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
			base_thread->choose(0);
			std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
			REQUIRE(snap->can_be_migrated());
			globals new_globals = new_story->new_globals_from_snapshot(*snap);
			runner  new_thread  = new_story->new_runner_from_snapshot(*snap, new_globals);
			THEN("Migrated visit counts. Unreachable node has visit, new node has no")
			{
				content = new_thread->getall();
				REQUIRE(content == "A\ncatch\n1 1 0\nOh.\n");
			}
		}
	}
}
