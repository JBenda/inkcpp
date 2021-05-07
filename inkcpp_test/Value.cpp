#include "catch.hpp"

#include "../inkcpp/string_table.h"
#include "../inkcpp/list_table.h"
#include "../inkcpp/random.h"
#include "../inkcpp/output.h"
#include "../inkcpp/executioner.h"
#include "../shared/private/command.h"

using ink::runtime::internal::value;
using ink::runtime::internal::value_type;
using ink::runtime::internal::string_table;
using ink::runtime::internal::list_table;
using ink::runtime::internal::prng;
using stream = ink::runtime::internal::stream<128>;
using ink::runtime::internal::executer;
using eval_stack = ink::runtime::internal::eval_stack<28, false>;
using ink::Command;

void cp_str(char* dst, const char* src) {
	while(*src) { *dst++ = *src++; }
	*dst = 0;
}

SCENARIO("compare concatenated values")
{
	string_table str_table;
	list_table lst_table{};
	prng rng;
	executer ops(str_table, lst_table, rng);
	eval_stack stack;
	GIVEN("just single strings")
	{
		const char str_1[] = "Hello World!";
		const char str_1_again[] = "Hello World!";
		const char str_2[] = "Bye World!";
		WHEN("equal")
		{
			stack.push(value{}.set<value_type::string>(str_1));
			stack.push(value{}.set<value_type::string>(str_1_again));
			ops(Command::IS_EQUAL, stack);
			value res = stack.pop();
			THEN("== results in true")
			{
				REQUIRE(res.type() == value_type::boolean);
				REQUIRE(res.get<value_type::boolean>() == true);
			}
		}
		WHEN("not equal")
		{
			stack.push(value{}.set<value_type::string>(str_1));
			stack.push(value{}.set<value_type::string>(str_2));
			ops(Command::IS_EQUAL, stack);
			value res = stack.pop();
			THEN("== results in false")
			{
				REQUIRE(res.type() == value_type::boolean);
				REQUIRE(res.get<value_type::boolean>() == false);
			}
		}
	}
	GIVEN("string and numbers")
	{
		stream out{};
		char* str_hello = str_table.create(6);
		cp_str(str_hello, "hello");
		char* str_5hello = str_table.create(7);
		cp_str(str_5hello, "5hello");
		char* str_4 = str_table.create(2);
		cp_str(str_4, "4");
		char* str_32_4 = str_table.create(33);
		for (int i = 0; i < 32; ++i) { str_32_4[i] = '4'; }
		str_32_4[32] = 0;

		int int_4 = 4;
		int int_45 = 45;
		WHEN("concatenated string representation matches (2 fields)")
		{
			stack.push(value{}.set<value_type::int32>(int_4));
			stack.push(value{}.set<value_type::string>(str_5hello));
			ops(Command::ADD, stack);
			stack.push(value{}.set<value_type::int32>(int_45));
			stack.push(value{}.set<value_type::string>(str_hello));
			ops(Command::ADD, stack);
			ops(Command::IS_EQUAL, stack);
			value res = stack.pop();
			THEN("== returns true")
			{
				REQUIRE(res.type() == value_type::boolean);
				REQUIRE(res.get<value_type::boolean>() == true);
			}
		}
		WHEN("concatenated string representation match (many fields)")
		{
			stack.push(value{}.set<value_type::string>(str_4));
			for (int i = 0; i < 31; ++i) {
				stack.push(value{}.set<value_type::int32>(int_4));
				ops(Command::ADD, stack);
			}
			stack.push(value{}.set<value_type::string>(str_32_4));
			ops(Command::IS_EQUAL, stack);
			value res = stack.pop();
			THEN("== results true")
			{
				REQUIRE(res.type() == value_type::boolean);
				REQUIRE(res.get<value_type::boolean>() == true);
			}
		}
		WHEN("concatenated string representation won't match")
		{
			stack.push(value{}.set<value_type::int32>(int_45));
			stack.push(value{}.set<value_type::string>(str_5hello));
			ops(Command::ADD, stack);
			stack.push(value{}.set<value_type::int32>(int_4));
			stack.push(value{}.set<value_type::string>(str_hello));
			ops(Command::ADD, stack);
			ops(Command::IS_EQUAL, stack);
			value res = stack.pop();
			THEN("== returns false")
			{
				REQUIRE(res.type() == value_type::boolean);
				REQUIRE(res.get<value_type::boolean>() == false);
			}
		}
	}
	GIVEN("numbers")
	{
		int i5 = 5;
		int i8 = 8;
		float f5 = 5.f;
		WHEN("numbers are same")
		{
			stack.push(value{}.set<value_type::int32>(i8));
			stack.push(value{}.set<value_type::int32>(i8));
			ops(Command::IS_EQUAL, stack);
			value res1 = stack.pop();
			stack.push(value{}.set<value_type::float32>(f5));
			stack.push(value{}.set<value_type::float32>(f5));
			ops(Command::IS_EQUAL, stack);
			value res2 = stack.pop();
			THEN("== returns true")
			{
				REQUIRE(res1.type() == value_type::boolean);
				REQUIRE(res1.get<value_type::boolean>() == true);
				REQUIRE(res2.type() == value_type::boolean);
				REQUIRE(res2.get<value_type::boolean>() == true);
			}
		}
		WHEN("numbers equal, but different encoding")
		{
			stack.push(value{}.set<value_type::int32>(i5));
			stack.push(value{}.set<value_type::float32>(f5));
			ops(Command::IS_EQUAL, stack);
			value res = stack.pop();
			THEN("== returns true")
			{
				REQUIRE(res.type() == value_type::boolean);
				REQUIRE(res.get<value_type::boolean>() == true);
			}
		}
		WHEN("numbers value and encoding differs")
		{
			stack.push(value{}.set<value_type::float32>(f5));
			stack.push(value{}.set<value_type::int32>(i8));
			ops(Command::IS_EQUAL, stack);
			value res = stack.pop();
			THEN("== returns false")
			{
				REQUIRE(res.type() == value_type::boolean);
				REQUIRE(res.get<value_type::boolean>() == false);
			}
		}
		WHEN("calculate with float and int (5.,8)")
		{
			stack.push(value{}.set<value_type::float32>(f5));
			stack.push(value{}.set<value_type::int32>(i8));
			THEN("adding results 13.")
			{
				ops(Command::ADD, stack);
				value res = stack.pop();
				REQUIRE(res.type() == value_type::float32);
				REQUIRE(res.get<value_type::float32>() == 13.f);
			}
			THEN("dividing results in 0.625")
			{
				ops(Command::DIVIDE, stack);
				value res = stack.pop();
				REQUIRE(res.type() == value_type::float32);
				REQUIRE(res.get<value_type::float32>() == 0.625f);
			}
		}
	}
}
