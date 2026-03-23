#include "catch.hpp"

#include "../inkcpp/hungarian_solver.h"

SCENARIO("find best assigments", "[hungarian]")
{
	GIVEN("Example 1")
	{
		// clang-format off
		float cost[] = {
			8/**/, 5    , 9    ,
			4    , 2    , 4/**/,
			7    , 3/**/, 8    ,
		};
		// clang-format on
		int   matches[3];
		float total_cost = hungarian_solver(cost, matches, 3);
		CHECK(total_cost == 15.f);
		CHECK(matches[0] == 0);
		CHECK(matches[1] == 2);
		CHECK(matches[2] == 1);
	}
	GIVEN("Example 2")
	{
		// clang-format off
		float cost[] = {
			108    , 150    , 122/**/,
			125    , 135/**/, 148    ,
			150/**/, 175    , 250    ,
		};
		// clang-format off
		int matches[3];
		float total_cost = hungarian_solver(cost, matches, 3);
		CHECK(total_cost == 407);
		CHECK(matches[0] == 2);
		CHECK(matches[1] == 1);
		CHECK(matches[2] == 0);
	}
}
