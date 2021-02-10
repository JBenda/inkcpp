#include "catch.hpp"

#include "../inkcpp/value.h"
#include "../inkcpp/string_table.h"
#include "../inkcpp/output.h"

using value = ink::runtime::internal::value;
using data_type = ink::runtime::internal::data_type;
using string_table = ink::runtime::internal::string_table;
using stream = ink::runtime::internal::stream<128>;

void cp_str(char* dst, const char* src) {
	while(*src) { *dst++ = *src++; }
	*dst = 0;
}

SCENARIO("compare concatenated values")
{
	GIVEN("just single strings")
	{
		const char str_1[] = "Hello World!";
		const char str_1_again[] = "Hello World!";
		const char str_2[] = "Bye World!";
		WHEN("equal")
		{
			value v1(str_1);
			value v2(str_1_again);
			value res = v1 == v2;
			THEN("== results in true")
			{
				REQUIRE(res.get_data_type() == data_type::int32);
				REQUIRE(res.get<int32_t>() == 1);
			}
		}
		WHEN("not equal")
		{
			value v1(str_1);
			value v2(str_2);
			value res = v1 == v2;
			THEN("== results in false")
			{
				REQUIRE(res.get_data_type() == data_type::int32);
				REQUIRE(res.get<int32_t>() == 0);
			}
		}
	}
	GIVEN("string and numbers")
	{
		string_table str_table;
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
			value v1 = value::add(value(int_4), value(str_5hello), out, str_table);
			value v2 = value::add(value(int_45), value(str_hello), out, str_table);
			value res = v1 == v2;
			THEN("== returns true")
			{
				REQUIRE(res.get_data_type() == data_type::int32);
				REQUIRE(res.get<int32_t>() == 1);
			}
		}
		WHEN("concatenated string representation match (many fields)")
		{
			value v1 = value(str_4);
			for (int i = 0; i < 31; ++i) {
				v1 = value::add(v1, value(int_4), out, str_table);
			}
			value v2 = value(str_32_4);
			value res = v1 == v2;
			THEN("== results true")
			{
				REQUIRE(res.get_data_type() == data_type::int32);
				REQUIRE(res.get<int32_t>() == 1);
			}
		}
		WHEN("concatenated string representation won't match")
		{
			value v1 = value::add(value(int_45), value(str_5hello), out, str_table);
			value v2 = value::add(value(int_4),value(str_hello), out, str_table);
			value res = v1 == v2;
			THEN("== returns false")
			{
				REQUIRE(res.get_data_type() == data_type::int32);
				REQUIRE(res.get<int32_t>() == 0);
			}
		}
	}
}
