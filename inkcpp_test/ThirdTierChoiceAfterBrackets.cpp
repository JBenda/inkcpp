#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO(
    "a story with a bracketed choice as a second choice, and then a third choice, chooses properly",
    "[choices]"
)
{
  GIVEN("a story with brackets and nested choices")
  {
    inklecate("ink/ThirdTierChoiceAfterBracketsStory.ink", "ThirdTierChoiceAfterBracketsStory.tmp");
    ink::compiler::run(
        "ThirdTierChoiceAfterBracketsStory.tmp", "ThirdTierChoiceAfterBracketsStory.bin"
    );
    auto   ink    = story::from_file("ThirdTierChoiceAfterBracketsStory.bin");
    runner thread = ink->new_runner();

    WHEN("start thread")
    {
      THEN("thread doesn't error")
      {
	thread->getall();
	thread->has_choices();
	thread->choose(0);
	thread->getall();
	thread->has_choices();
	thread->choose(0);
	thread->getall();
	thread->has_choices();
	thread->choose(0);
	thread->getall();
	thread->has_choices();
      }
    }
  }
}
