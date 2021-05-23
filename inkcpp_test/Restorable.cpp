#include "catch.hpp"

#include "../inkcpp/collections/restorable.h"

using ink::runtime::internal::restorable;

SCENARIO("a restorable collection can operate like a stack", "[restorable]")
{
	GIVEN("an empty restorable collection")
	{
		// Create the collection
		constexpr size_t size = 128;
		int buffer[size];
		auto collection = restorable(buffer, size);

		// Lambdas
		auto isNull = [](const int&) { return false; };

		THEN("it should have zero size") REQUIRE(collection.size(isNull) == 0);

		WHEN("we push ten elements")
		{
			for (int i = 0; i < 10; i++)
				collection.push(i);

			THEN("the count should be ten") REQUIRE(collection.size(isNull) == 10);

			THEN("The top should be nine") REQUIRE(collection.top(isNull) == 9);

			THEN("We can iterate forward")
			{
				int check = 0;
				collection.for_each([&check](int& elem) { REQUIRE(elem == check++); }, isNull);
			}

			THEN("We can iterate backward")
			{
				int check = 10;
				collection.reverse_for_each([&check](int& elem) { REQUIRE(elem == --check); }, isNull);
			}

			THEN("Pop five elements")
			{
				REQUIRE(collection.pop(isNull) == 9);
				REQUIRE(collection.pop(isNull) == 8);
				REQUIRE(collection.pop(isNull) == 7);
				REQUIRE(collection.pop(isNull) == 6);

				REQUIRE(collection.top(isNull) == 5);
			}
		}
	}
}

void VerifyStack(restorable<int>& stack, const std::vector<int>& expected)
{
	THEN("it should match the expected array in both directions")
	{
		auto isNull = [](const int& e) { return e == -1; };

		// Check
		REQUIRE(stack.size(isNull) == expected.size());

		// Iterate and make sure each element matches
		int counter = 0;
		stack.for_each([&counter, expected](const int& elem) {
			REQUIRE(counter < expected.size());
			REQUIRE(counter >= 0);
			REQUIRE(expected[counter] == elem);
			counter++;
		}, isNull);

		// Make sure we hit every element in the expected vector
		REQUIRE(counter == expected.size());

		// Try the other direction
		stack.reverse_for_each([&counter, expected](const int& elem) {
			counter--;
			REQUIRE(counter >= 0);
			REQUIRE(expected[counter] == elem);
		}, isNull);
		REQUIRE(counter == 0);
	}
}

template<typename ElementType>
void RestoreAndVerifyStack(restorable<ElementType>& stack, const std::vector<ElementType>& expected)
{
	WHEN("the stack is restored")
	{
		// Restore stack
		stack.restore();

		VerifyStack(stack, expected);
	}
}

template<typename ElementType, typename NullifyCallback>
void ForgetAndVerifyStack(restorable<ElementType>& stack, const std::vector<ElementType>& expected, NullifyCallback nullify)
{
	WHEN("the save state is forgotten")
	{
		// Forget save point
		stack.forget(nullify);

		VerifyStack(stack, expected);
	}
}

SCENARIO("a collection can be restored no matter how many times you push or pop", "[restorable]")
{
	// Create the collection
	constexpr size_t size = 128;
	int buffer[size];
	auto collection = restorable(buffer, size);

	// Lambdas (we'll use negative one for null)
	auto isNull = [](const int& elem) { return elem == -1; };
	auto nullify = [](int& elem) { elem = -1; };

	GIVEN("a stack with five items that has been saved")
	{
		// Create five elements
		std::vector<int> expected;
		for (int i = 0; i < 5; i++)
		{
			collection.push(i);
			expected.push_back(i);
		}

		// Create save point
		collection.save();

		WHEN("more elements are pushed")
		{
			collection.push(10);
			collection.push(11);
			RestoreAndVerifyStack(collection, expected);
		}

		WHEN("elements are popped")
		{
			collection.pop(isNull); 
			collection.pop(isNull);
			RestoreAndVerifyStack(collection, expected);
		}

		WHEN("elements are popped and pushed")
		{
			collection.pop(isNull);
			collection.pop(isNull);
			collection.push(100);
			collection.push(200);
			collection.push(300);
			RestoreAndVerifyStack(collection, expected);
		}

		WHEN("all elements are popped")
		{
			for (int i = 0; i < 5; i++) {
				collection.pop(isNull);
			}
			REQUIRE(collection.size(isNull) == 0);
			RestoreAndVerifyStack(collection, expected);

			THEN("More are pushed")
			{
				collection.push(100); collection.push(200);
				VerifyStack(collection, { 100, 200 });
			}
		}
	}
}

SCENARIO("saving does not disrupt iteration", "[restorable]")
{
	// Create the collection
	constexpr size_t size = 128;
	int buffer[size];
	auto collection = restorable(buffer, size);

	// Lambdas (we'll use negative one for null)
	auto isNull = [](const int& elem) { return elem == -1; };
	auto nullify = [](int& elem) { elem = -1; };

	GIVEN("a stack with five items that has been saved")
	{
		// Create five elements
		std::vector<int> expected;
		for (int i = 0; i < 5; i++)
		{
			collection.push(i);
			expected.push_back(i);
		}

		// Create save point
		collection.save();

		WHEN("elements are pushed")
		{
			collection.push(10); expected.push_back(10);
			collection.push(20); expected.push_back(20);
			VerifyStack(collection, expected);
		}

		WHEN("elements are popped")
		{
			collection.pop(isNull); expected.pop_back();
			collection.pop(isNull); expected.pop_back();
			VerifyStack(collection, expected);
		}

		WHEN("elements are popped and pushed")
		{
			collection.pop(isNull); expected.pop_back();
			collection.pop(isNull); expected.pop_back();
			collection.push(10); expected.push_back(10);
			collection.push(20); expected.push_back(20);
			VerifyStack(collection, expected);
		}
	}

	GIVEN("an empty saved stack")
	{
		std::vector<int> expected;
		collection.save();

		WHEN("elements are pushed")
		{
			collection.push(10); expected.push_back(10);
			collection.push(20); expected.push_back(20);
			VerifyStack(collection, expected);
		}

		WHEN("elements are pushed then popped")
		{
			collection.push(10); expected.push_back(10);
			collection.push(20); expected.push_back(20);
			collection.pop(isNull); expected.pop_back();
			collection.pop(isNull); expected.pop_back();
			VerifyStack(collection, expected);
		}
	}
}

SCENARIO("save points can be forgotten", "[restorable]")
{
	// Create the collection
	constexpr size_t size = 128;
	int buffer[size];
	auto collection = restorable(buffer, size);

	// Lambdas (we'll use negative one for null)
	auto isNull = [](const int& elem) { return elem == -1; };
	auto nullify = [](int& elem) { elem = -1; };

	GIVEN("a stack with five items that has been saved")
	{
		// Create five elements
		std::vector<int> expected;
		for (int i = 0; i < 5; i++)
		{
			collection.push(i);
			expected.push_back(i);
		}

		// Create save point
		collection.save();

		WHEN("elements are pushed")
		{
			collection.push(10); expected.push_back(10);
			collection.push(20); expected.push_back(20);
			ForgetAndVerifyStack(collection, expected, nullify);
		}

		WHEN("elements are popped")
		{
			collection.pop(isNull); expected.pop_back();
			collection.pop(isNull); expected.pop_back();
			ForgetAndVerifyStack(collection, expected, nullify);
		}

		WHEN("elements are popped and pushed")
		{
			collection.pop(isNull); expected.pop_back();
			collection.pop(isNull); expected.pop_back();
			collection.push(10); expected.push_back(10);
			collection.push(20); expected.push_back(20);
			collection.push(30); expected.push_back(30);
			ForgetAndVerifyStack(collection, expected, nullify);
		}
	}
}
