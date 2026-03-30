#include "catch.hpp"

#include "../inkcpp/hungarian_solver.h"

#include <iostream>

namespace ink::runtime::internal
{
struct MatchListValues {
	const char* const* names;
	const int*         values;
	size_t             length;
};

float** cost_matrix(const MatchListValues& rh, const MatchListValues& lh, float drop_panelty);
float   d_contains(const int lh[2], const int rh[2], const int* matches);
float   d_value(int lh, int rh, int lh_range[2], int rh_range[2]);
float   d_label(const char* lh, const char* rh);
} // namespace ink::runtime::internal

SCENARIO("santy check distance functions", "[list_match]")
{
	SECTION("Labels")
	{
		SECTION("jaro_simularity")
		{
			GIVEN("Two Stings")
			{
				float j = jaro_simularity("FAREMVIEL", "FARMVILLE");
				CHECK_THAT(j, Catch::Matchers::WithinAbs(0.88, 0.01));
			}
			GIVEN("Two Strings in different Casing, no impact ignore casing")
			{
				float j = jaro_simularity("FAREMVIEL", "farmville");
				CHECK_THAT(j, Catch::Matchers::WithinAbs(0.88, 0.01));
			}
			GIVEN("Two strings with fill characters, small impact")
			{
				float j = jaro_simularity("FAREMVIEL", "FARMV_IL-LE");
				CHECK_THAT(j, Catch::Matchers::WithinAbs(0.83, 0.01));
			}
		}
		SECTION("jaro_winkler_simularity")
		{
			GIVEN("Two Strings wih without prefix")
			{
				float j  = jaro_simularity("ZFAREMVIEL", "YFARMVILLE");
				float jw = jaro_winkler_simularity("ZFAREMVIEL", "YFARMVILLE");
				CHECK_THAT(jw, Catch::Matchers::WithinAbs(j, 0.01));
			}
			GIVEN("Two Strings with prefix")
			{
				float j  = jaro_simularity("FAREMVIEL", "FARMVILLE");
				float jw = jaro_winkler_simularity("FAREMVIEL", "FARMVILLE");
				CHECK(j < jw);
			}
		}
	}
	SECTION("Values")
	{
		int range1[] = {0, 20};
		int range2[] = {5, 35};
		GIVEN("Same Value")
		{
			float d = ink::runtime::internal::d_value(5, 5, range1, range1);
			CHECK(d == 0);
			d = ink::runtime::internal::d_value(5, 5, range1, range2);
			CHECK(d == 0);
		}
		GIVEN("Different Value")
		{
			float d1 = ink::runtime::internal::d_value(10, 20, range1, range1);
			float d2 = ink::runtime::internal::d_value(10, 20, range2, range2);
			float d3 = ink::runtime::internal::d_value(10, 20, range1, range2);
			CHECK(d3 == 0); // there are both in the center of their respected range
			CHECK(d1 > d2); // same absolute distance in bigger range is a smaller distance
		}
	}
	SECTION("Sets")
	{
		GIVEN("Equal Sets")
		{
			int   lh[]      = {5, 10};
			int   rh[]      = {5, 10};
			int   matches[] = {0, 0, 0, 0, 0, 5, 6, 7, 8, 9};
			float d         = ink::runtime::internal::d_contains(lh, rh, matches);
			CHECK_THAT(d, Catch::Matchers::WithinAbs(1, 0.001));
		}
		GIVEN("Dropped Values")
		{
			int   lh[]      = {5, 10};
			int   rh[]      = {5, 8};
			int   matches[] = {0, 0, 0, 0, 0, 5, -1, -1, 6, 7};
			float d         = ink::runtime::internal::d_contains(lh, rh, matches);
			CHECK_THAT(d, Catch::Matchers::WithinAbs(0.6, 0.001));
		}
		GIVEN("New Values")
		{
			int   lh[]      = {5, 8};
			int   rh[]      = {5, 10};
			int   matches[] = {0, 0, 0, 0, 0, 5, 6, 7};
			float d         = ink::runtime::internal::d_contains(lh, rh, matches);
			CHECK_THAT(d, Catch::Matchers::WithinAbs(0.6, 0.001));
		}
		GIVEN("Swapped Values")
		{
			int   lh[]      = {5, 10};
			int   rh[]      = {5, 10};
			int   matches[] = {0, 0, 0, 0, 0, 5, 9, 6, 8, 7};
			float d         = ink::runtime::internal::d_contains(lh, rh, matches);
			CHECK_THAT(d, Catch::Matchers::WithinAbs(1, 0.001));
		}
		GIVEN("Changed Values")
		{
			int   lh[]      = {5, 10};
			int   rh[]      = {5, 10};
			int   matches[] = {0, 0, 0, 0, 0, 5, 9, -1, -1, -1};
			float d         = ink::runtime::internal::d_contains(lh, rh, matches);
			CHECK_THAT(d, Catch::Matchers::WithinAbs(0.25, 0.001));
		}
	}
}

SCENARIO("find best assigments", "[list_match][hungarian]")
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
	GIVEN("With Example 1 Threshold") {
		// clang-format off
		float cost[] = {
			8/**/, 5    , 9    ,
			4    , 2    , 4/**/,
			7    , 3/**/, 8    ,
		};
		// clang-format on
		int   matches[3];
		float total_cost = hungarian_solver(cost, matches, 3, 5);
		CHECK(total_cost == 15.f);
		CHECK(matches[0] == -1);
		CHECK(matches[1] == 2);
		CHECK(matches[2] == 1);
	}
}
