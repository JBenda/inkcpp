#include "catch.hpp"

#include "../inkcpp/simple_restorable_stack.h"

using ink::runtime::internal::simple_restorable_stack;

template<typename T, ink::size_t N>
using fixed_restorable_stack = ink::runtime::internal::managed_restorable_stack<T,false,N>;

template<typename T, int SIZE>
void stack_matches(const simple_restorable_stack<T>& stack, T (&expected)[SIZE])
{
	// Iterate stack and check that it matches the expected array
	int count = 0;
	const int* iter = nullptr;
	while (stack.iter(iter)) {
		REQUIRE(count < SIZE);
		REQUIRE(*iter == expected[SIZE - count - 1]);
		count++;
	}
}

SCENARIO("simple_restorable_stack can be pushed and popped", "[stack]") {

	GIVEN("An empty stack") {
		fixed_restorable_stack<int, 10> stack(~0);

		WHEN("items are added") {
			stack.push(1);
			stack.push(2);
			stack.push(3);

			THEN("the size has increased") {
				REQUIRE(!stack.empty());
				REQUIRE(stack.size() == 3);
			}

			THEN("the correct item is at the top") {
				REQUIRE(stack.top() == 3);
			}

			THEN("we can iterate the stack") {
				int expected[] = { 1, 2, 3 };
				stack_matches(stack, expected);
			}

			WHEN("they are popped") {
				int pop = stack.pop();

				THEN("the correct item is popped") {
					REQUIRE(pop == 3);
				}

				THEN("there are fewer items") {
					REQUIRE(stack.size() == 2);
				}

				THEN("there is a new item on top") {
					REQUIRE(stack.top() == 2);
				}

				THEN("we can still iterate") {
					int expected[] = { 1, 2 };
					stack_matches(stack, expected);
				}
			}
		}
	}
}

SCENARIO("simple_restorable_stack supports save/restore", "[stack]") {
	GIVEN("a stack with a few items that has been saved") {
		fixed_restorable_stack<int, 20> stack(~0);

		stack.push(1); 
		stack.push(2);
		stack.push(3);
		stack.save();

		auto check_restore = [&stack]() {
			WHEN("we restore the stack") {
				stack.restore();

				THEN("we should be back to having 4 items") {
					REQUIRE(stack.size() == 3);
				}

				THEN("they should match exactly the original items") {
					int expected[] = { 1, 2, 3 };
					stack_matches(stack, expected);
				}
			}
		};

		WHEN("new items are pushed") {
			stack.push(4);

			THEN("the pushed item is on top") {
				REQUIRE(stack.top() == 4);
			}
			THEN("the size has increased") {
				REQUIRE(stack.size() == 4);
			}

			WHEN("the state is restored") {
				stack.restore();

				THEN("the original item is back on top") {
					REQUIRE(stack.size() == 3);
					REQUIRE(stack.top() == 3);
				}
			}

			WHEN("the state is finalized") {
				stack.forget();

				THEN("the new item is still on top") {
					REQUIRE(stack.size() == 4);
					REQUIRE(stack.top() == 4);
				}
			}
		}

		WHEN("items are popped") {
			stack.pop(); stack.pop();

			THEN("the stack has shrunk") {
				REQUIRE(stack.size() == 1);
				REQUIRE(stack.top() == 1);
			}

			WHEN("more items are pushed") {
				stack.push(90);
				stack.push(91);

				THEN("the stack has the correct size") {
					REQUIRE(stack.size() == 3);
					REQUIRE(stack.top() == 91);
				}

				THEN("iteration only covers items that ought to be in the stack") {
					int expected[] = { 1, 90, 91 };
					stack_matches(stack, expected);
				}

				THEN("we can pop all items from the stack") {
					REQUIRE(stack.pop() == 91);
					REQUIRE(stack.pop() == 90);
					REQUIRE(stack.pop() == 1);
					REQUIRE(stack.empty());

					// Check that the stack can restore
					check_restore();

					THEN("we can add more items to the stack")
					{
						// Push more items
						stack.push(100);
						stack.push(101);
						int expected[] = { 100, 101 };
						stack_matches(stack, expected);

						// Check that we can still restore
						check_restore();
					}
				}

				// Check that the stack can restore
				check_restore();

				THEN("we can finalize the stack") {
					stack.forget();

					int expected[] = { 1, 90, 91 };
					stack_matches(stack, expected);
				}

				WHEN("we pop them back off") {
					stack.pop(); stack.pop();

					THEN("the correct entry is on top") {
						REQUIRE(stack.top() == 1);
						REQUIRE(stack.size() == 1);
					}
				}
			}
		}
	}

	GIVEN("a stack with one entry that has been saved")
	{
		fixed_restorable_stack<int, 10> stack(~0);
		stack.push(0);
		stack.save();

		WHEN("an item is popped, pushed, and popped again")
		{
			stack.pop();
			stack.push(1);
			stack.pop();

			THEN("the stack should be considered 'empty'")
			{
				REQUIRE(stack.size() == 0); 
				REQUIRE(stack.empty());
			}
		}

		WHEN("an item is popped and pushed and then we forget()")
		{
			stack.pop();
			stack.push(1);
			stack.pop();
			stack.forget();

			THEN("the stack should be empty")
			{
				REQUIRE(stack.size() == 0);
				REQUIRE(stack.empty());
			}

		}
	}
}
