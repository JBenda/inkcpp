#include "catch.hpp"

#include <../runner_impl.h>
#include <choice.h>
#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

SCENARIO("Observer", "[variables][observer]")
{
	GIVEN("a story which changes variables")
	{
		auto ink     = story::from_file(INK_TEST_RESOURCE_DIR "ObserverStory.bin");
		auto globals = ink->new_globals();
		auto thread  = ink->new_runner(globals).cast<internal::runner_impl>();

		std::stringstream debug;
		thread->set_debug_enabled(&debug);

		WHEN("Run without observers")
		{
			CHECK(thread->getall() == "hello line 1 1 hello line 2 5 test line 3 5\n");
		}
		WHEN("Run with observers read only, with specific type")
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

			CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
			CHECK(var1_cnt == 2);
			CHECK(var2_cnt == 2);
		}
		WHEN("Run with generic observer")
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

			CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
			CHECK(var1_cnt == 2);
			CHECK(var2_cnt == 2);
		}
		WHEN("Bind multiple observer to same variables")
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

			CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
			CHECK(var1_cnt == 4);
		}
		WHEN("Run with missmatching type")
		{
			auto var1 = [](uint32_t i) {
			};
			CHECK_THROWS(globals->observe("var1", var1));
		}
		WHEN("Just get pinged")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt]() {
        var1_cnt++;
			};
			globals->observe("var1", var1);
			std::string out = thread->getall();

			CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
			CHECK(var1_cnt == 2);
		}
		WHEN("call with new and old value")
		{
			int  var1_cnt = 0;
			auto var1     = [&var1_cnt](int32_t i, ink::optional<int32_t> o_i) {
        if (var1_cnt++ == 0) {
					CHECK(i == 1);
					CHECK_FALSE(o_i.has_value());
				} else {
					CHECK(i == 5);
					CHECK(o_i.has_value());
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
          CHECK(o_v.has_value());
          CHECK(o_v.value().type == value::Type::String);
          std::string str2(o_v.value().get<value::Type::String>());
          CHECK(str2 == "hello");
        }
			};

			globals->observe("var1", var1);
			globals->observe("var2", var2);
			std::string out = thread->getall();

			CHECK(out == "hello line 1 1 hello line 2 5 test line 3 5\n");
			CHECK(var1_cnt == 2);
			CHECK(var2_cnt == 2);
		}
		WHEN("Changing Same value at runtime")
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

			CHECK(8 == globals->get<int32_t>("var1").value());
			CHECK(out == "hello line 1 1 hello line 2 8 test line 3 8\n");
			CHECK(var1_cnt == 3);
		}
		WHEN("Changing Sam value at bind time")
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

			CHECK(5 == globals->get<int32_t>("var1").value());
			CHECK(out == "hello line 1 8 hello line 2 5 test line 3 5\n");
			CHECK(var1_cnt == 3);
		}
		WHEN("Changing Same value multiple times")
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

			CHECK(5 == globals->get<int32_t>("var1").value());
			CHECK(out == "hello line 1 10 hello line 2 5 test line 3 5\n");
			CHECK(var1_cnt == 4);
		}
		WHEN("Changing Other value")
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

			CHECK(out == "hello line 1 1 didum line 2 5 test line 3 5\n");
			CHECK(var1_cnt == 2);
			CHECK(var2_cnt == 3);
		}
	}
}
