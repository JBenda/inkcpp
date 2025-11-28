#include "catch.hpp"
#include "snapshot.h"
#include "../snapshot_impl.h"

#include <memory>
#include <story.h>
#include <globals.h>
#include <choice.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("Simple isolated migration tests.")
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
		REQUIRE(base_thread->num_global_tags() == 2);
		REQUIRE(base_thread->get_global_tag(0) == std::string("test:migration"));
		REQUIRE(base_thread->get_global_tag(1) == std::string("flavor:base"));
		REQUIRE(base_thread->num_knot_tags() == 1);
		REQUIRE(base_thread->get_knot_tag(0) == std::string("knot:Main"));
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
	GIVEN("Simple story with changed temporary variables.")
	{
		std::unique_ptr<story> new_story{story::from_file(INK_TEST_RESOURCE_DIR "MigrationTemp.bin")};
		WHEN("Just Run the new story.")
		{
			globals     new_globals = new_story->new_globals();
			runner      new_thread  = new_story->new_runner(new_globals);
			std::string content     = new_thread->getall();
			REQUIRE(new_thread->has_choices());
			REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
			REQUIRE(new_thread->get_current_knot() == 0x25e83b84);
			new_thread->choose(0);
			REQUIRE(new_thread->get_current_knot() == 0x25e83b84);
			content = new_thread->getall();
			REQUIRE(new_thread->get_current_knot() == 0x25e83b84);
			REQUIRE(content == "A\ncatch\n2 - 3\n1 -1 0\nOh.\n");
		}
		WHEN("Run base story and load new story.")
		{
			std::string content = base_thread->getall();
			REQUIRE(base_thread->has_choices());
			REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
			REQUIRE(base_thread->get_current_knot() == 0x25e83b84);
			base_thread->choose(0);
			REQUIRE(base_thread->get_current_knot() == 0x25e83b84);
			std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
			REQUIRE(snap->can_be_migrated());
			globals new_globals = new_story->new_globals_from_snapshot(*snap);
			runner  new_thread  = new_story->new_runner_from_snapshot(*snap, new_globals);
			THEN("Transfared old temporary variable, and kept default from new one.")
			{
				REQUIRE(new_thread->get_current_knot() == 0x25e83b84);
				content = new_thread->getall();
				REQUIRE(content == "A\ncatch\n5 - 6\n1 -1 0\nOh.\n");
			}
		}
	}
	GIVEN("Simple story with other knot/global tags.")
	{
		std::unique_ptr<story> new_story{story::from_file(INK_TEST_RESOURCE_DIR "MigrationKnotTags.bin")
		};
		WHEN("Just Run the new story.")
		{
			globals     new_globals = new_story->new_globals();
			runner      new_thread  = new_story->new_runner(new_globals);
			std::string content     = new_thread->getall();
			REQUIRE(new_thread->has_choices());
			REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
			REQUIRE(new_thread->num_global_tags() == 2);
			REQUIRE(new_thread->get_global_tag(0) == std::string("test:migration"));
			REQUIRE(new_thread->get_global_tag(1) == std::string("flavor:changed"));
			REQUIRE(new_thread->num_knot_tags() == 1);
			REQUIRE(new_thread->get_knot_tag(0) == std::string("knot:different"));
		}
		WHEN("Run base story and load new story.")
		{
			std::string content = base_thread->getall();
			REQUIRE(base_thread->has_choices());
			REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
			REQUIRE(base_thread->num_global_tags() == 2);
			REQUIRE(base_thread->get_global_tag(0) == std::string("test:migration"));
			REQUIRE(base_thread->get_global_tag(1) == std::string("flavor:base"));
			REQUIRE(base_thread->num_knot_tags() == 1);
			REQUIRE(base_thread->get_knot_tag(0) == std::string("knot:Main"));
			base_thread->choose(0);
			std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
			REQUIRE(snap->can_be_migrated());
			globals new_globals = new_story->new_globals_from_snapshot(*snap);
			runner  new_thread  = new_story->new_runner_from_snapshot(*snap, new_globals);
			THEN("Got new global/knot tags")
			{
				REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
				REQUIRE(new_thread->num_global_tags() == 2);
				REQUIRE(new_thread->get_global_tag(0) == std::string("test:migration"));
				REQUIRE(new_thread->get_global_tag(1) == std::string("flavor:changed"));
				REQUIRE(new_thread->num_knot_tags() == 1);
				REQUIRE(new_thread->get_knot_tag(0) == std::string("knot:different"));
			}
		}
	}
}
