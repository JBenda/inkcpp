#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

static constexpr const char* OUTPUT_PART_1 = "Once upon a time...\n";
static constexpr const char* OUTPUT_PART_2
    = "There were two choices.\nThey lived happily ever after.\n";
static constexpr size_t CHOICE = 0;

SCENARIO("run inklecate 1.1.1 story")
{
	auto compiler = GENERATE("inklecate", "inky");
	GIVEN(compiler)
	{
		auto input_file = std::string(INK_TEST_RESOURCE_DIR "simple-1.1.1-") + compiler + ".json";
		ink::compiler::run(input_file.c_str(), "simple.bin");
		auto   ink    = story::from_file("simple.bin");
		runner thread = ink->new_runner();

		THEN("Expect normal output")
		{
			REQUIRE(thread->getall() == OUTPUT_PART_1);
			REQUIRE(thread->has_choices());
			REQUIRE(thread->num_choices() == 2);
			thread->choose(CHOICE);
			REQUIRE(thread->getall() == OUTPUT_PART_2);
		}
	}
}
