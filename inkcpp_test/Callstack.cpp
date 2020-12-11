#include "catch.hpp"

#include "../inkcpp/stack.h"

using ink::hash_t;
using ink::thread_t;
using ink::runtime::internal::value;
using ink::runtime::internal::frame_type;

const hash_t X = ink::hash_string("X");
const hash_t Y = ink::hash_string("Y");
const hash_t Z = ink::hash_string("Z");

SCENARIO("threading with the callstack", "[callstack]")
{
	GIVEN("a callstack with a few temporary variables")
	{
		// Create the stack
		auto stack = ink::runtime::internal::stack<50>();

		// Set X and Y temporary variables
		stack.set(X, 100);
		stack.set(Y, 200);

		WHEN("there is a fork and more is pushed")
		{
			thread_t thread = stack.fork_thread();

			THEN("old variables aren't accessible")
			{
				REQUIRE(stack.get(X) == nullptr);
				REQUIRE(stack.get(Y) == nullptr);
			}

			// Push something onto the thread
			stack.set(X, 200);
			REQUIRE((int)*stack.get(X) == 200);

			WHEN("when that thread ends")
			{
				stack.complete_thread(thread);

				WHEN("we collapse to the thread")
				{
					stack.collapse_to_thread(thread);

					THEN("we should have the value from that thread")
					{
						REQUIRE((int)*stack.get(X) == 200);
					}
				}

				WHEN("we collapse to the main thraed")
				{
					stack.collapse_to_thread(~0);

					THEN("we should have the value from the original thread")
					{
						REQUIRE((int)*stack.get(X) == 100);
						REQUIRE((int)*stack.get(Y) == 200);
					}
				}

				THEN("we should be able to access original thread values")
				{
					REQUIRE((int)*stack.get(X) == 100);
					REQUIRE((int)*stack.get(Y) == 200);
				}

				WHEN("we push more on the main thread")
				{
					stack.set(Z, 500);

					THEN("collapsing to the thread shouldn't have the values anymore")
					{
						stack.collapse_to_thread(thread);
						REQUIRE(stack.get(Z) == nullptr);
					}

					WHEN("we start a second thread that closes")
					{
						thread_t thread2 = stack.fork_thread();
						stack.set(X, 999);
						stack.complete_thread(thread2);

						THEN("we can still collapse to the main thread")
						{
							stack.collapse_to_thread(~0);
							REQUIRE((int)*stack.get(X) == 100);
							REQUIRE((int)*stack.get(Y) == 200);
						}

						THEN("we can still collapse to the first thread")
						{
							stack.collapse_to_thread(thread);
							REQUIRE((int)*stack.get(X) == 200);
							REQUIRE(stack.get(Z) == nullptr);
						}
					}
				}
			}

			WHEN("that thread also forks a thread")
			{
				thread_t thread2 = stack.fork_thread();

				// Put something on this thread
				stack.set(X, 999);

				WHEN("that inner thread and outer thread complete")
				{
					stack.complete_thread(thread2);
					stack.complete_thread(thread);

					WHEN("we collapse to the inner thread")
					{
						stack.collapse_to_thread(thread2);

						THEN("we should have the value from that thread")
						{
							REQUIRE((int)*stack.get(X) == 999);
						}
					}

					WHEN("we collapse to the outer thread")
					{
						stack.collapse_to_thread(thread);

						THEN("we should have the value from that thread")
						{
							REQUIRE((int)*stack.get(X) == 200);
						}
					}

					WHEN("we collapse to the main thraed")
					{
						stack.collapse_to_thread(~0);

						THEN("we should have the value from the original thread")
						{
							REQUIRE((int)*stack.get(X) == 100);
							REQUIRE((int)*stack.get(Y) == 200);
						}
					}
				}
			}
		}
	}

	GIVEN("a callstack with a single tunnel pushed")
	{
		// Create the stack
		auto stack = ink::runtime::internal::stack<50>();

		// Set X and Y temporary variables
		stack.set(X, 100);
		stack.set(Y, 200);

		// Push a tunnel
		stack.push_frame(505, frame_type::tunnel);

		// Push some more temps
		stack.set(X, 101);
		stack.set(Y, 201);

		WHEN("a thread is forked")
		{
			thread_t thread = stack.fork_thread();

			WHEN("that thread does a tunnel return")
			{
				frame_type type;
				auto offset = stack.pop_frame(&type);
				
				THEN("that thread should be outside the tunnel")
				{
					REQUIRE(type == frame_type::tunnel);
					REQUIRE(offset == 505);
				}
			}
		}
	}
}
