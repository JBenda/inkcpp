#include "catch.hpp"

#include "..//inkcpp/array.h"

using namespace ink;
using ink::runtime::internal::allocated_restorable_array;

typedef allocated_restorable_array<uint32_t> test_array;

SCENARIO("a restorable array can hold values", "[array]")
{
	GIVEN("an empty array")
	{
		const ink::size_t length = 10;
		test_array array = test_array(length, ~0);

		THEN("the default values should be zero")
		{
			for (ink::size_t i = 0; i < length; i++)
			{
				REQUIRE(array[i] == 0);
			}
		}

		WHEN("we assign a value")
		{
			array.set(3, 15);

			THEN("the value should be set")
			{
				REQUIRE(array[3] == 15);
			}

			THEN("the other values should be zero still")
			{
				for (ink::size_t i = 0; i < length; i++)
				{
					if (i == 3)
						continue;

					REQUIRE(array[i] == 0);
				}
			}
		}
	}
}

SCENARIO("a restorable array can save/restore/forget", "[array]")
{
	GIVEN("a saved array with a few values")
	{
		// Load up the array
		test_array array = test_array(5, ~0);
		array.set(0, 0);
		array.set(1, 1);
		array.set(2, 2);
		array.set(3, 3);
		array.set(4, 4);

		// Save its state
		array.save();

		WHEN("we change some values")
		{
			array.set(0, 10);
			array.set(1, 11);

			THEN("the new values should be returned")
			{
				REQUIRE(array[0] == 10);
				REQUIRE(array[1] == 11);
			}

			THEN("old values should be returned when not changed")
			{
				REQUIRE(array[2] == 2);
				REQUIRE(array[3] == 3);
				REQUIRE(array[4] == 4);
			}

			WHEN("we restore the array")
			{
				array.restore();

				THEN("we should get our old values back")
				{
					REQUIRE(array[0] == 0);
					REQUIRE(array[1] == 1);
				}

				WHEN("we save again")
				{
					array.save();

					THEN("we shouldn't get the weird values")
					{
						REQUIRE(array[0] == 0);
						REQUIRE(array[1] == 1);
					}
				}
			}

			WHEN("we forget the save")
			{
				array.forget();

				THEN("we should still have the new values")
				{
					REQUIRE(array[0] == 10);
					REQUIRE(array[1] == 11);
				}
			}
		}
	}
}