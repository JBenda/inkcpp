#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story with bracketed choices and spaces can choose correctly", "[choices]")
{
  GIVEN("a story with line breaks")
  {
    inklecate("ink/ChoiceBracketStory.ink", "ChoiceBracketStory.tmp");
    ink::compiler::run("ChoiceBracketStory.tmp", "ChoiceBracketStory.bin");
    auto   ink    = story::from_file("ChoiceBracketStory.bin");
    runner thread = ink->new_runner();
    thread->getall();
    WHEN("start thread")
    {
      THEN("thread has choices")
      {
	thread->getall();
	REQUIRE(thread->has_choices());
      }
      WHEN("choose choice 1")
      {
	thread->choose(0);
	thread->getall();
	THEN("still has choices")
	{
	  thread->getall();
	  REQUIRE(thread->has_choices());
	}
      }
      WHEN("choose choice 2")
      {
	thread->choose(1);
	thread->getall();
	THEN("still has choices")
	{
	  REQUIRE(thread->has_choices());
	}
      }
    }
  }
}
