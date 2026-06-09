#include "catch.hpp"

#include "../inkcpp/hungarian_solver.h"
#include <story.h>
#include <runner.h>
#include <globals.h>
#include <snapshot.h>

using namespace ink::runtime;

namespace ink::runtime::internal
{
struct MatchListValues {
	const char* const* names;
	const int*         values;
	size_t             length;
};

float** cost_matrix(const MatchListValues& rh, const MatchListValues& lh, float drop_panelty);
float   d_contains(const size_t lh[2], const size_t rh[2], const int* matches);
float   d_value(int lh, int rh, int lh_range[2], int rh_range[2]);
float   d_label(const char* lh, const char* rh);
} // namespace ink::runtime::internal

SCENARIO("santy check distance functions", "[list-matching][unit][internals]")
{
	GIVEN("two strings for jaro similarity comparison")
	{
		WHEN("the strings differ only by transpositions (FAREMVIEL vs FARMVILLE)")
		{
			float j = ink::algorithms::jaro_simularity("FAREMVIEL", "FARMVILLE");

			THEN("the similarity is approximately 0.88")
			{
				CHECK_THAT(j, Catch::Matchers::WithinAbs(0.88, 0.01));
			}
		}

		WHEN("the strings differ only in casing (FAREMVIEL vs farmville)")
		{
			float j = ink::algorithms::jaro_simularity("FAREMVIEL", "farmville");

			THEN("the similarity is the same as the case-sensitive comparison, approximately 0.88")
			{
				CHECK_THAT(j, Catch::Matchers::WithinAbs(0.88, 0.01));
			}
		}

		WHEN("one string contains fill characters (FAREMVIEL vs FARMV_IL-LE)")
		{
			float j = ink::algorithms::jaro_simularity("FAREMVIEL", "FARMV_IL-LE");

			THEN("the similarity is slightly lower, approximately 0.83")
			{
				CHECK_THAT(j, Catch::Matchers::WithinAbs(0.83, 0.01));
			}
		}
	}

	GIVEN("two strings for jaro-winkler similarity comparison")
	{
		WHEN("the strings share no common prefix (ZFAREMVIEL vs YFARMVILLE)")
		{
			float j  = ink::algorithms::jaro_simularity("ZFAREMVIEL", "YFARMVILLE");
			float jw = ink::algorithms::jaro_winkler_simularity("ZFAREMVIEL", "YFARMVILLE");

			THEN("jaro-winkler equals jaro since there is no shared prefix bonus")
			{
				CHECK_THAT(jw, Catch::Matchers::WithinAbs(j, 0.01));
			}
		}

		WHEN("the strings share a common prefix (FAREMVIEL vs FARMVILLE)")
		{
			float j  = ink::algorithms::jaro_simularity("FAREMVIEL", "FARMVILLE");
			float jw = ink::algorithms::jaro_winkler_simularity("FAREMVIEL", "FARMVILLE");

			THEN("jaro-winkler is higher than jaro due to the prefix bonus") { CHECK(j < jw); }
		}
	}

	GIVEN("two integer values and their ranges for value distance comparison")
	{
		int range1[] = {0, 20};
		int range2[] = {5, 35};

		WHEN("the values are the same")
		{
			THEN("the distance is zero regardless of the range")
			{
				float d = ink::runtime::internal::d_value(5, 5, range1, range1);
				CHECK(d == 0);
				d = ink::runtime::internal::d_value(5, 5, range1, range2);
				CHECK(d == 0);
			}
		}

		WHEN("the values are different")
		{
			float d1 = ink::runtime::internal::d_value(10, 20, range1, range1);
			float d2 = ink::runtime::internal::d_value(10, 20, range2, range2);
			float d3 = ink::runtime::internal::d_value(10, 20, range1, range2);

			THEN("the same absolute distance in a bigger range results in a smaller relative distance")
			{
				CHECK(d3 == 0); // both values are at the centre of their respective ranges
				CHECK(d1 > d2);
			}
		}
	}

	GIVEN("two sets and a match array for containment distance comparison")
	{
		WHEN("the sets are equal")
		{
			ink::size_t lh[]      = {5, 10};
			ink::size_t rh[]      = {5, 10};
			int         matches[] = {0, 0, 0, 0, 0, 5, 6, 7, 8, 9};
			float       d         = ink::runtime::internal::d_contains(lh, rh, matches);

			THEN("the distance is zero") { CHECK_THAT(d, Catch::Matchers::WithinAbs(0, 0.001)); }
		}

		WHEN("the right-hand set has dropped values")
		{
			ink::size_t lh[]      = {5, 10};
			ink::size_t rh[]      = {5, 8};
			int         matches[] = {0, 0, 0, 0, 0, 5, -1, -1, 6, 7};
			float       d         = ink::runtime::internal::d_contains(lh, rh, matches);

			THEN("the distance reflects the dropped elements")
			{
				CHECK_THAT(d, Catch::Matchers::WithinAbs(0.4, 0.001));
			}
		}

		WHEN("the right-hand set has new values")
		{
			ink::size_t lh[]      = {5, 8};
			ink::size_t rh[]      = {5, 10};
			int         matches[] = {0, 0, 0, 0, 0, 5, 6, 7};
			float       d         = ink::runtime::internal::d_contains(lh, rh, matches);

			THEN("the distance reflects the new elements")
			{
				CHECK_THAT(d, Catch::Matchers::WithinAbs(0.4, 0.001));
			}
		}

		WHEN("the sets have swapped values")
		{
			ink::size_t lh[]      = {5, 10};
			ink::size_t rh[]      = {5, 10};
			int         matches[] = {0, 0, 0, 0, 0, 5, 9, 6, 8, 7};
			float       d         = ink::runtime::internal::d_contains(lh, rh, matches);

			THEN("the distance is zero since order does not matter")
			{
				CHECK_THAT(d, Catch::Matchers::WithinAbs(0, 0.001));
			}
		}

		WHEN("the sets have changed values")
		{
			ink::size_t lh[]      = {5, 10};
			ink::size_t rh[]      = {5, 10};
			int         matches[] = {0, 0, 0, 0, 0, 5, 9, -1, -1, -1};
			float       d         = ink::runtime::internal::d_contains(lh, rh, matches);

			THEN("the distance reflects the changed elements")
			{
				CHECK_THAT(d, Catch::Matchers::WithinAbs(0.75, 0.001));
			}
		}
	}
}

SCENARIO("find best assigments", "[list-matching][unit][internals]")
{
	GIVEN("a 3x3 cost matrix (example 1)")
	{
		// clang-format off
		float cost[] = {
			8/**/, 5    , 9    ,
			4    , 2    , 4/**/,
			7    , 3/**/, 8    ,
		};
		// clang-format on
		int   matches[3];
		float total_cost = ink::algorithms::hungarian_solver(cost, matches, 3);

		THEN("the optimal assignment has the correct total cost and match indices")
		{
			CHECK(total_cost == 15.f);
			CHECK(matches[0] == 0);
			CHECK(matches[1] == 2);
			CHECK(matches[2] == 1);
		}
	}

	GIVEN("a 3x3 cost matrix (example 2)")
	{
		// clang-format off
		float cost[] = {
			108    , 150    , 122/**/,
			125    , 135/**/, 148    ,
			150/**/, 175    , 250    ,
		};
		// clang-format on
		int   matches[3];
		float total_cost = ink::algorithms::hungarian_solver(cost, matches, 3);

		THEN("the optimal assignment has the correct total cost and match indices")
		{
			CHECK(total_cost == 407);
			CHECK(matches[0] == 2);
			CHECK(matches[1] == 1);
			CHECK(matches[2] == 0);
		}
	}

	GIVEN("a 3x3 cost matrix with a drop threshold applied (example 1 with threshold)")
	{
		// clang-format off
		float cost[] = {
			8/**/, 5    , 9    ,
			4    , 2    , 4/**/,
			7    , 3/**/, 8    ,
		};
		// clang-format on
		int   matches[3];
		float total_cost = ink::algorithms::hungarian_solver(cost, matches, 3, 5);

		THEN("the first element is dropped and the remaining assignments are optimal")
		{
			CHECK(total_cost == 15.f);
			CHECK(matches[0] == -1);
			CHECK(matches[1] == 2);
			CHECK(matches[2] == 1);
		}
	}
}

SCENARIO("Simple List Migration stories", "[list-matching][migration][integration]")
{
	GIVEN("a story split across two versions with list extensions and a typo fix")
	{
		std::unique_ptr<story> ink_a{story::from_file(INK_TEST_RESOURCE_DIR "ListMatchStoryA.bin")};
		std::unique_ptr<story> ink_b{story::from_file(INK_TEST_RESOURCE_DIR "ListMatchStoryB.bin")};
		globals                globals_a = ink_a->new_globals();
		runner                 thread_a  = ink_a->new_runner(globals_a);

		WHEN("the old story is advanced past the first choice and a snapshot is taken")
		{
			REQUIRE(thread_a->getline() == "You are currently at Flor, Balcony\n");
			REQUIRE(thread_a->has_choices());
			thread_a->choose(0);
			std::unique_ptr<snapshot> snap{thread_a->create_snapshot()};
			REQUIRE(snap->can_be_migrated());

			THEN("the old story continues with the original list names")
			{
				CHECK(
				    thread_a->getall()
				    == "More\nYou are still at Flor, Balcony - all posibilities are Flor, Balcony, Kitchen, Garden\n"
				);
			}

			AND_WHEN("the snapshot is loaded into the new story version")
			{
				auto        globals_b = ink_b->new_globals_from_snapshot(*snap);
				auto        thread_b  = ink_b->new_runner_from_snapshot(*snap, globals_b);
				std::string out       = thread_b->getall();

				THEN("the new story resumes with the corrected list names and extended possibilities")
				{
					CHECK(
					    out
					    == "More\nYou are still at Floor, Balcony - all posibilities are Kitchen, Street, Floor, Balcony, Livingroom, Garden\n"
					);
				}
			}
		}
	}
}
