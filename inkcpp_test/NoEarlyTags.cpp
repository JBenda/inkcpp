#include "../inkcpp_cl/test.h"
#include "catch.hpp"

#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

SCENARIO("Story with tags and glues", "[glue, tags]")
{
  GIVEN("lines intersected with tags and glue")
  {
    inklecate("ink/NoEarlyTags.ink", "NoEarlyTags.tmp");
    ink::compiler::run("NoEarlyTags.tmp", "NoEarlyTags.bin");
    auto ink = story::from_file("NoEarlyTags.bin");
    auto thread = ink->new_runner();
    WHEN("no glue")
    {
      std::string out = thread->getline();
      REQUIRE(out == "Hey there, nice to meet you!\n");
      REQUIRE(thread->num_tags() == 2);
    }
    WHEN("glue")
    {
      thread->getline();
      std::string out = thread->getline();
      REQUIRE(out == "Hey, I'm Hey and this is YOU, nice to meet you too!I'm Do! Most people can't pronounce it, just think 'Kee-vah\".\n");
      REQUIRE(thread->num_tags() == 5);
    }
  }
}

