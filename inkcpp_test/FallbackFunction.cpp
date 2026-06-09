#include "catch.hpp"

#include <system.h>
#include <story.h>
#include <runner.h>
#include <globals.h>
#include <compiler.h>

#include <cmath>

using namespace ink::runtime;

SCENARIO(
    "run a story with external function and fallback function", "[external-functions][runtime]"
)
{
	GIVEN("a story with two external functions, one with fallback")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "FallBack.bin")};
		runner                 thread = ink->new_runner();

		WHEN("both external functions are bound")
		{
			int  cnt_sqrt = 0;
			auto fn_sqrt  = [&cnt_sqrt](double x) -> double {
        ++cnt_sqrt;
        return static_cast<int>(sqrt(x));
			};
			int  cnt_greeting = 0;
			auto fn_greeting  = [&cnt_greeting]() -> const char* {
        ++cnt_greeting;
        return "Hohooh";
			};

			thread->bind("sqrt", fn_sqrt);
			thread->bind("greeting", fn_greeting);
			std::string out;
			REQUIRE_NOTHROW(out = thread->getall());

			THEN("the bound greeting is used and both functions are called the correct number of times")
			{
				REQUIRE(
				    out
				    == "Hohooh ! A small demonstration of my power:\nMath 4 * 4 = 16, stunning i would "
				       "say\n"
				);
				REQUIRE(cnt_sqrt == 2);
				REQUIRE(cnt_greeting == 1);
			}
		}

		WHEN("only the function without a fallback is bound")
		{
			int  cnt_sqrt = 0;
			auto fn_sqrt  = [&cnt_sqrt](double x) -> double {
        ++cnt_sqrt;
        return static_cast<int>(sqrt(x));
			};

			thread->bind("sqrt", fn_sqrt);
			std::string out;
			REQUIRE_NOTHROW(out = thread->getall());

			THEN("the fallback greeting is used and sqrt is called the correct number of times")
			{
				REQUIRE(
				    out
				    == "Hello ! A small demonstration of my power:\nMath 4 * 4 = 16, stunning i would say\n"
				);
				REQUIRE(cnt_sqrt == 2);
			}
		}

		WHEN("no functions are bound")
		{
			THEN("running the story throws an exception for the missing non-fallback function")
			{
				std::string out;
				REQUIRE_THROWS_AS(out = thread->getall(), ink::ink_exception);
			}
		}
	}
}
