#include "catch.hpp"
#include "system.h"
#include "../runner_impl.h"

#include <choice.h>
#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

SCENARIO("Observer", "[observer][globals][runtime]")
{
	GIVEN("a story which changes variables")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "ObserverStory.bin")};
		auto                   globals = ink->new_globals();
		auto                   thread  = ink->new_runner(globals).cast<internal::runner_impl>();

		std::stringstream debug;
		thread->set_debug_enabled(&debug);

		WHEN("the story runs with no observers registered")
		{
			std::string out = thread->getall();

			THEN("the output is correct")
			{
				CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
			}
		}

		WHEN("typed observers are registered for var1 and var2")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt](int32_t i) {
        if (var1_cnt++ == 0) {
					CHECK(i == 1);
				} else {
					CHECK(i == 5);
				}
			};
			int  var2_cnt = 0;
			auto var2     = [&var2_cnt](const char* s) {
        std::string str(s);
        if (var2_cnt++ == 0) {
					CHECK(str == "hello");
				} else {
					CHECK(str == "test");
				}
			};

			globals->observe("var1", var1);
			globals->observe("var2", var2);
			std::string out = thread->getall();

			THEN("the output is correct and each observer is called the expected number of times")
			{
				CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
				CHECK(var1_cnt == 2);
				CHECK(var2_cnt == 2);
			}
		}

		WHEN("generic value observers are registered for var1 and var2")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt](value v) {
        CHECK(v.type == value::Type::Int32);
        if (var1_cnt++ == 0) {
          CHECK(v.get<value::Type::Int32>() == 1);
        } else {
          CHECK(v.get<value::Type::Int32>() == 5);
        }
			};
			int  var2_cnt = 0;
			auto var2     = [&var2_cnt](value v) {
        CHECK(v.type == value::Type::String);
        std::string str(v.get<value::Type::String>());
        if (var2_cnt++ == 0) {
          CHECK(str == "hello");
        } else {
          CHECK(str == "test");
        }
			};

			globals->observe("var1", var1);
			globals->observe("var2", var2);
			std::string out = thread->getall();

			THEN("the output is correct and each observer is called the expected number of times")
			{
				CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
				CHECK(var1_cnt == 2);
				CHECK(var2_cnt == 2);
			}
		}

		WHEN("the same observer is bound twice to the same variable")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt](int32_t i) {
        if (var1_cnt++ < 2) {
					CHECK(i == 1);
				} else {
					CHECK(i == 5);
				}
			};
			globals->observe("var1", var1);
			globals->observe("var1", var1);
			std::string out = thread->getall();

			THEN("the observer is called twice per change and the output is correct")
			{
				CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
				CHECK(var1_cnt == 4);
			}
		}

		WHEN("an observer with a mismatching type is registered")
		{
			auto var1 = [](uint32_t) {
			};

			THEN("registering the observer throws an exception")
			{
				CHECK_THROWS_AS(globals->observe("var1", var1), ink::ink_exception);
			}
		}

		WHEN("a no-argument ping observer is registered for var1")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt]() {
        var1_cnt++;
			};
			globals->observe("var1", var1);
			std::string out = thread->getall();

			THEN("the output is correct and the observer is pinged once per change")
			{
				CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
				CHECK(var1_cnt == 2);
			}
		}

		WHEN("observers receiving both new and old values are registered")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt](int32_t i, ink::optional<int32_t> o_i) {
        if (var1_cnt++ == 0) {
          CHECK(i == 1);
          CHECK_FALSE(o_i.has_value());
        } else {
          CHECK(i == 5);
					REQUIRE(o_i.has_value());
					CHECK(o_i.value() == 1);
				}
			};
			int  var2_cnt = 0;
			auto var2     = [&var2_cnt](value v, ink::optional<value> o_v) {
        CHECK(v.type == value::Type::String);
        std::string str(v.get<value::Type::String>());
        if (var2_cnt++ == 0) {
          CHECK(str == "hello");
          CHECK_FALSE(o_v.has_value());
        } else {
          CHECK(str == "test");
					REQUIRE(o_v.has_value());
					CHECK(o_v.value().type == value::Type::String);
					std::string str2(o_v.value().get<value::Type::String>());
					CHECK(str2 == "hello");
				}
			};

			globals->observe("var1", var1);
			globals->observe("var2", var2);
			std::string out = thread->getall();

			THEN(
			    "each observer receives the correct new and old values and is called the expected number "
			    "of times"
			)
			{
				CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
				CHECK(var1_cnt == 2);
				CHECK(var2_cnt == 2);
			}
		}

		WHEN("an observer modifies the same variable it is observing at runtime")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt, &globals](int32_t i) {
        ++var1_cnt;
        if (var1_cnt == 1) {
					CHECK(i == 1);
				} else if (var1_cnt == 2) {
					CHECK(i == 5);
					globals->set<int32_t>("var1", 8);
				} else if (var1_cnt == 3) {
					CHECK(i == 8);
				}
			};
			globals->observe("var1", var1);
			std::string out = thread->getall();

			THEN("the modification is reflected in the output and the observer is called an extra time")
			{
				CHECK(globals->get<int32_t>("var1").value() == 8);
				CHECK(out == "hello line 1 1 hello line 2 8 test line 3 8\n");
				CHECK(var1_cnt == 3);
			}
		}

		WHEN("an observer modifies the same variable it is observing at bind time")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt, &globals](int32_t i) {
        ++var1_cnt;
        if (var1_cnt == 1) {
					CHECK(i == 1);
					globals->set<int32_t>("var1", 8);
				} else if (var1_cnt == 2) {
					CHECK(i == 8);
				} else if (var1_cnt == 3) {
					CHECK(i == 5);
				}
			};
			globals->observe("var1", var1);
			std::string out = thread->getall();

			THEN("the bind-time modification propagates and the story uses the modified value first")
			{
				CHECK(globals->get<int32_t>("var1").value() == 5);
				CHECK(out == "hello line 1 8 hello line 2 5 test line 3 5\n");
				CHECK(var1_cnt == 3);
			}
		}

		WHEN("an observer modifies the same variable multiple times in a chain")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt, &globals](int32_t i) {
        ++var1_cnt;
        if (var1_cnt == 1) {
					CHECK(i == 1);
					globals->set<int32_t>("var1", 8);
				} else if (var1_cnt == 2) {
					CHECK(i == 8);
					globals->set<int32_t>("var1", 10);
				} else if (var1_cnt == 3) {
					CHECK(i == 10);
				} else if (var1_cnt == 4) {
					CHECK(i == 5);
				}
			};
			globals->observe("var1", var1);
			std::string out = thread->getall();

			THEN("each chained modification triggers the observer and the story reflects the final value")
			{
				CHECK(globals->get<int32_t>("var1").value() == 5);
				CHECK(out == "hello line 1 10 hello line 2 5 test line 3 5\n");
				CHECK(var1_cnt == 4);
			}
		}

		WHEN("an observer for var1 modifies a different variable var2")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt, &globals](int32_t i) {
        if (var1_cnt++ == 0) {
					CHECK(i == 1);
				} else {
					CHECK(i == 5);
					globals->set<const char*>("var2", "didum");
				}
			};
			int  var2_cnt = 0;
			auto var2     = [&var2_cnt]() {
        ++var2_cnt;
			};

			globals->observe("var1", var1);
			globals->observe("var2", var2);
			std::string out = thread->getall();

			THEN(
			    "the cross-variable modification is reflected in the output and var2's observer is "
			    "triggered the extra time"
			)
			{
				CHECK(out == "hello line 1 1 didum line 2 5 test line 3 5\n");
				CHECK(var1_cnt == 2);
				CHECK(var2_cnt == 3);
			}
		}
	}
}
