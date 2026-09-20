#include "catch.hpp"

#include "..//inkcpp/array.h"

using namespace ink;
using ink::runtime::internal::allocated_restorable_array;
using ink::runtime::internal::managed_instances;

typedef allocated_restorable_array<uint32_t> test_array;

namespace
{
struct instance_payload {
	int value;
};
} // namespace

SCENARIO("a restorable array can hold values", "[array][unit][internals]")
{
	GIVEN("an empty array")
	{
		const ink::size_t length = 10;
		test_array        array  = test_array(length, 0U, ~0U);

		THEN("the default values should be zero")
		{
			for (ink::size_t i = 0; i < length; i++) {
				REQUIRE(array[i] == 0);
			}
		}

		WHEN("we assign a value")
		{
			array.set(3, 15);

			THEN("the value should be set") { REQUIRE(array[3] == 15); }

			THEN("the other values should be zero still")
			{
				for (ink::size_t i = 0; i < length; i++) {
					if (i == 3)
						continue;

					REQUIRE(array[i] == 0);
				}
			}
		}
	}
}

SCENARIO("a restorable array can save/restore/forget", "[array][unit][internals]")
{
	GIVEN("a saved array with a few values")
	{
		// Load up the array
		test_array array = test_array(5, 0U, ~0U);
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

SCENARIO(
    "a static managed_instances hands out a fixed number of instances", "[array][unit][internals]"
)
{
	GIVEN("a static managed_instances with capacity 3")
	{
		managed_instances<instance_payload, 3> instances;

		THEN("it starts out empty")
		{
			auto stats = instances.statistics();
			REQUIRE(stats.capacity == 3);
			REQUIRE(stats.size == 0);
		}

		WHEN("we push up to capacity")
		{
			instances.push().value = 1;
			instances.push().value = 2;
			instances.push().value = 3;

			THEN("statistics report the full, unchanged capacity")
			{
				auto stats = instances.statistics();
				REQUIRE(stats.capacity == 3);
				REQUIRE(stats.size == 3);
			}

			THEN("pushing beyond capacity fails") { REQUIRE_THROWS(instances.push()); }
		}
	}
}

SCENARIO(
    "a dynamic managed_instances grows in chunks without moving old data",
    "[array][unit][internals]"
)
{
	GIVEN("a dynamic managed_instances with initial capacity 5")
	{
		managed_instances<instance_payload, -5> instances;

		THEN("it starts out at the configured initial capacity")
		{
			auto stats = instances.statistics();
			REQUIRE(stats.capacity == 5);
			REQUIRE(stats.size == 0);
		}

		WHEN("we fill the initial chunk")
		{
			instance_payload& first = instances.push();
			first.value             = 42;
			for (int i = 1; i < 5; ++i) {
				instances.push().value = i;
			}

			THEN("no growth was necessary yet")
			{
				auto stats = instances.statistics();
				REQUIRE(stats.capacity == 5);
				REQUIRE(stats.size == 5);
			}

			WHEN("we push past the initial chunk, forcing growth")
			{
				instances.push().value = 100;

				THEN("capacity grows by the expected chunk size")
				{
					auto stats = instances.statistics();
					REQUIRE(stats.capacity == 7);
					REQUIRE(stats.size == 6);
				}

				THEN("the previously handed out reference is still valid and unchanged")
				{
					REQUIRE(first.value == 42);
				}

				WHEN("we push until the second chunk is also exhausted")
				{
					instances.push().value = 101;

					THEN("statistics reflect the still unchanged, now full capacity")
					{
						auto stats = instances.statistics();
						REQUIRE(stats.capacity == 7);
						REQUIRE(stats.size == 7);
					}

					WHEN("we push once more, forcing a third chunk")
					{
						instances.push().value = 102;

						THEN("capacity grows again by the expected chunk size")
						{
							auto stats = instances.statistics();
							REQUIRE(stats.capacity == 10);
							REQUIRE(stats.size == 8);
						}

						THEN("the very first reference handed out is still valid and unchanged")
						{
							REQUIRE(first.value == 42);
						}
					}
				}
			}
		}
	}
}

SCENARIO(
    "clear() on a dynamic managed_instances reuses previously allocated memory",
    "[array][unit][internals]"
)
{
	GIVEN("a dynamic managed_instances with a few pushed instances")
	{
		managed_instances<instance_payload, -5> instances;
		instance_payload&                       first = instances.push();
		first.value                                   = 7;
		instances.push().value                        = 8;

		WHEN("we clear it")
		{
			instances.clear();

			THEN("size resets but capacity is unchanged")
			{
				auto stats = instances.statistics();
				REQUIRE(stats.size == 0);
				REQUIRE(stats.capacity == 5);
			}

			WHEN("we push again")
			{
				instance_payload& reused = instances.push();

				THEN("the same memory is handed out again") { REQUIRE(&reused == &first); }
			}
		}
	}
}

SCENARIO(
    "a dynamic managed_instances with a single initial slot still grows correctly",
    "[array][unit][internals]"
)
{
	GIVEN("a dynamic managed_instances configured with an initial capacity of one")
	{
		managed_instances<instance_payload, -1> instances;

		THEN("it starts out at the configured initial capacity")
		{
			auto stats = instances.statistics();
			REQUIRE(stats.capacity == 1);
			REQUIRE(stats.size == 0);
		}

		WHEN("we push well past the initial capacity")
		{
			instance_payload& first = instances.push();
			first.value             = 1;
			for (int i = 2; i <= 5; ++i) {
				instances.push().value = i;
			}

			THEN("every push succeeded and grew capacity to keep up")
			{
				auto stats = instances.statistics();
				REQUIRE(stats.size == 5);
				REQUIRE(stats.capacity >= 5);
			}

			THEN("the first handed out reference is still valid and unchanged")
			{
				REQUIRE(first.value == 1);
			}
		}
	}
}
