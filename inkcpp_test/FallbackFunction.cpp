#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <system.h>
#include <story.h>
#include <runner.h>
#include <globals.h>
#include <compiler.h>

#include <cmath>

using namespace ink::runtime;

SCENARIO("run a story with external function and fallback function", "[external function]")
{
  GIVEN("story with two external functions, one with fallback")
  {
    inklecate("ink/FallBack.ink", "FallBack.tmp");
    ink::compiler::run("FallBack.tmp", "FallBack.bin");
    auto ink = story::from_file("FallBack.bin");
    runner thread = ink->new_runner();
    
    WHEN("bind both external functions")
    {
      int cnt_sqrt = 0;
      auto fn_sqrt = [&cnt_sqrt](int x)->int{ ++cnt_sqrt; return sqrt(x); };
      int cnt_greeting = 0;
      auto fn_greeting = [&cnt_greeting]()->const char*{++cnt_greeting; return "Hohooh"; };
      
      thread->bind("sqrt", fn_sqrt);
      thread->bind("greeting", fn_greeting);
      
      std::string out;
      REQUIRE_NOTHROW(out = thread->getall());
      THEN("Both function should be called the correct amount of times")
      {
        REQUIRE(out == "Hohooh ! A small demonstraion of my power:\n4 * 4 = 16, stunning i would say\n");
        REQUIRE(cnt_sqrt == 2);
        REQUIRE(cnt_greeting == 1);
      }
    }
    WHEN("only bind function without fallback")
    {
      int cnt_sqrt = 0;
      auto fn_sqrt = [&cnt_sqrt](int x)->int{++cnt_sqrt; return sqrt(x); };
      
      thread ->bind("sqrt", fn_sqrt);
      
      std::string out;
      REQUIRE_NOTHROW(out = thread->getall());;
      THEN("Sqrt should be falled twice, and uses default greeting")
      {
        REQUIRE(out == "Hello ! A small demonstraion of my power:\n4 * 4 = 16, stunning i would say\n");
        REQUIRE(cnt_sqrt == 2);
      }
    }
    WHEN("bind no function")
    {
      std::string out;
      REQUIRE_THROWS_AS(out = thread->getall(), ink::ink_exception);
    }
  }
}
